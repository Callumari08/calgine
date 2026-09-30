#include "physics_internal.h"

#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Body/BodyLock.h>

#include "calgine/core/physics/rigid_body.h"
#include "calgine/core/physics/collision_event.h"
#include "calgine/core/event_context.h"
#include "calgine/core/game_object.h"
#include "calgine/core/renderer/renderer.h"
#include "calgine/core/renderer/camera/camera_manager.h"
#include "calgine/core/log.h"

#include <chrono>
#include <cstdarg>
#include <thread>

namespace Calgine {

namespace {

constexpr JPH::uint max_bodies = 65536;
constexpr JPH::uint num_body_mutexes = 0; // 0 = Jolt picks a default
constexpr JPH::uint max_body_pairs = 65536;
// Large piles need several contact constraints per body; too few makes Jolt silently drop contacts.
constexpr JPH::uint max_contact_constraints = 65536;
constexpr size_t temp_allocator_size = 10 * 1024 * 1024;

void jolt_trace(const char* format, ...)
{
  va_list list;
  va_start(list, format);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), format, list);
  va_end(list);

  Log::get_engine_logger()->info("[Jolt] {}", buffer);
}

#ifdef JPH_ENABLE_ASSERTS
bool jolt_assert_failed(const char* expression, const char* message, const char* file, JPH::uint line)
{
  Log::get_engine_logger()->error("[Jolt] Assert failed {}:{}: ({}) {}", file, line, expression, message ? message : "");
  return true; // break into the debugger
}
#endif

glm::vec4 to_glm_colour(JPH::ColorArg colour)
{
  return glm::vec4(colour.r, colour.g, colour.b, colour.a) / 255.0f;
}

}

// ---- Debug renderer ----

#ifdef JPH_DEBUG_RENDERER
void PhysicsDebugRenderer::DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg colour)
{
  Renderer::get_instance().submit_line(to_glm(from), to_glm(to), to_glm_colour(colour));
}

void PhysicsDebugRenderer::DrawTriangle(JPH::RVec3Arg v1, JPH::RVec3Arg v2, JPH::RVec3Arg v3, JPH::ColorArg colour, ECastShadow)
{
  // Wireframe only.
  DrawLine(v1, v2, colour);
  DrawLine(v2, v3, colour);
  DrawLine(v3, v1, colour);
}
#endif

// ---- PhysicsWorld ----

PhysicsWorld::PhysicsWorld() = default;
PhysicsWorld::~PhysicsWorld() = default;

PhysicsWorld& PhysicsWorld::get_instance()
{
  static PhysicsWorld instance;
  return instance;
}

void PhysicsWorld::init()
{
  if (impl)
  {
    Log::get_engine_logger()->warn("PhysicsWorld::init() called twice, ignoring.");
    return;
  }

  Log::get_engine_logger()->info("Initializing Jolt Physics");

  JPH::RegisterDefaultAllocator();

  JPH::Trace = jolt_trace;
  JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = jolt_assert_failed;)

  JPH::Factory::sInstance = new JPH::Factory();
  JPH::RegisterTypes();

  impl = std::make_unique<Impl>();

  impl->temp_allocator = std::make_unique<JPH::TempAllocatorImplWithMallocFallback>(static_cast<JPH::uint>(temp_allocator_size));

  const int worker_threads = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
  impl->job_system = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, worker_threads);

  impl->system = std::make_unique<JPH::PhysicsSystem>();
  impl->system->Init(max_bodies, num_body_mutexes, max_body_pairs, max_contact_constraints,
                     impl->broad_phase_layer_interface, impl->object_vs_broad_phase_filter, impl->object_pair_filter);
  impl->system->SetContactListener(&impl->contact_queue);

#ifdef JPH_DEBUG_RENDERER
  impl->debug_renderer = std::make_unique<PhysicsDebugRenderer>();
#endif

  Log::get_engine_logger()->info("Jolt Physics ready ({} worker threads)", worker_threads);
}

void PhysicsWorld::shutdown()
{
  if (!impl)
    return;

  // Remove and destroy every body, and detach RigidBodies so their destructors don't touch the world later
  // (the static hierarchies are destroyed after the App).
  JPH::BodyInterface& body_interface = impl->body_interface();
  for (auto& [id, rigid_body] : impl->bodies)
  {
    const JPH::BodyID body_id(id);
    if (body_interface.IsAdded(body_id))
      body_interface.RemoveBody(body_id);
    body_interface.DestroyBody(body_id);

    rigid_body->body_id = RigidBody::invalid_body_id;
    rigid_body->in_simulation = false;
  }
  impl->bodies.clear();
  step_stats_callback = nullptr;

#ifdef JPH_DEBUG_RENDERER
  impl->debug_renderer.reset();
#endif

  impl->system.reset();
  impl->job_system.reset();
  impl->temp_allocator.reset();
  impl.reset();

  JPH::UnregisterTypes();
  delete JPH::Factory::sInstance;
  JPH::Factory::sInstance = nullptr;

  Log::get_engine_logger()->info("Jolt Physics shut down");
}

void PhysicsWorld::step(float delta_time, EventContext& event_context)
{
  if (!impl)
    return;

  using Clock = std::chrono::steady_clock;
  const auto to_ms = [](Clock::duration duration) { return std::chrono::duration<float, std::milli>(duration).count(); };

  PhysicsStepStats stats;
  const Clock::time_point step_start = Clock::now();

  // Copy first: RigidBody callbacks must never change the map, but be defensive anyway.
  std::vector<RigidBody*> rigid_bodies;
  rigid_bodies.reserve(impl->bodies.size());
  for (auto& [id, rigid_body] : impl->bodies)
    rigid_bodies.push_back(rigid_body);

  for (RigidBody* rigid_body : rigid_bodies)
    rigid_body->pre_step(delta_time);

  const Clock::time_point simulate_start = Clock::now();

  JPH::EPhysicsUpdateError error = impl->system->Update(delta_time, 1, impl->temp_allocator.get(), impl->job_system.get());
  if (error != JPH::EPhysicsUpdateError::None)
  {
    // Log when the kind of error changes, otherwise at most once every 5 seconds.
    const uint32_t flags = static_cast<uint32_t>(error);
    const auto now = std::chrono::steady_clock::now();
    if (flags != impl->last_logged_error || now - impl->last_error_log_time > std::chrono::seconds(5))
    {
      std::string reasons;
      if (flags & static_cast<uint32_t>(JPH::EPhysicsUpdateError::ManifoldCacheFull))      reasons += " manifold cache full;";
      if (flags & static_cast<uint32_t>(JPH::EPhysicsUpdateError::BodyPairCacheFull))      reasons += " body pair cache full;";
      if (flags & static_cast<uint32_t>(JPH::EPhysicsUpdateError::ContactConstraintsFull)) reasons += " contact constraints full;";
      Log::get_engine_logger()->warn("Jolt dropped some contacts this step:{} (too many touching bodies for the configured limits)", reasons);

      impl->last_logged_error = flags;
      impl->last_error_log_time = now;
    }
  }

  const Clock::time_point post_start = Clock::now();

  for (RigidBody* rigid_body : rigid_bodies)
    rigid_body->post_step();

  const Clock::time_point events_start = Clock::now();

  // Turn queued contacts into CollisionEvents on the main thread.
  for (const QueuedContact& contact : impl->contact_queue.take())
  {
    RigidBody* body_a = impl->find(contact.body_1);
    RigidBody* body_b = impl->find(contact.body_2);
    if (!body_a || !body_b)
      continue; // one of the bodies was removed in the meantime

    CollisionEvent event;
    event.submit_tick = TickType::fixed_update;
    event.type = contact.is_enter ? CollisionEvent::enter : CollisionEvent::exit;
    event.body_a = body_a;
    event.body_b = body_b;
    event.a = body_a->get_game_object();
    event.b = body_b->get_game_object();
    event.contact_point = contact.point;
    event.normal = contact.normal;

    event_context.submit(event);
    stats.collision_events++;
  }

  const Clock::time_point step_end = Clock::now();

  stats.pre_step_ms = to_ms(simulate_start - step_start);
  stats.simulate_ms = to_ms(post_start - simulate_start);
  stats.post_step_ms = to_ms(events_start - post_start);
  stats.events_ms = to_ms(step_end - events_start);
  stats.total_ms = to_ms(step_end - step_start);
  stats.bodies = static_cast<uint32_t>(impl->bodies.size());
  stats.active_bodies = impl->system->GetNumActiveBodies(JPH::EBodyType::RigidBody);

  last_step_stats = stats;
  total_steps++;

  if (step_stats_callback)
    step_stats_callback(stats);
}

void PhysicsWorld::debug_draw()
{
#ifdef JPH_DEBUG_RENDERER
  if (!impl || !debug_draw_enabled)
    return;

  if (CameraBehaviour* camera = CameraManager::get_instance().get_active_camera())
    impl->debug_renderer->SetCameraPos(to_jolt_r(camera->get_game_object()->get_world_position()));

  JPH::BodyManager::DrawSettings settings;
  settings.mDrawShape = true;
  settings.mDrawShapeWireframe = true;
  settings.mDrawShapeColor = JPH::BodyManager::EShapeColor::MotionTypeColor;

  impl->system->DrawBodies(settings, impl->debug_renderer.get());
  impl->debug_renderer->NextFrame();
#endif
}

void PhysicsWorld::set_gravity(const glm::vec3& gravity)
{
  if (!impl)
  {
    Log::get_engine_logger()->warn("PhysicsWorld::set_gravity called before the physics world was initialized.");
    return;
  }
  impl->system->SetGravity(to_jolt(gravity));
}

glm::vec3 PhysicsWorld::get_gravity() const
{
  if (!impl)
    return glm::vec3(0.0f, -9.81f, 0.0f);
  return to_glm(impl->system->GetGravity());
}

size_t PhysicsWorld::get_body_count() const
{
  return impl ? impl->bodies.size() : 0;
}

std::optional<RaycastHit> PhysicsWorld::raycast(const glm::vec3& origin, const glm::vec3& direction, float max_distance) const
{
  if (!impl || max_distance <= 0.0f || glm::length(direction) <= 0.0f)
    return std::nullopt;

  const glm::vec3 ray_vector = glm::normalize(direction) * max_distance;
  const JPH::RRayCast ray(to_jolt_r(origin), to_jolt(ray_vector));

  JPH::RayCastResult result;
  if (!impl->system->GetNarrowPhaseQuery().CastRay(ray, result))
    return std::nullopt;

  RaycastHit hit;
  hit.distance = result.mFraction * max_distance;
  const JPH::RVec3 point = ray.GetPointOnRay(result.mFraction);
  hit.point = to_glm(point);

  {
    JPH::BodyLockRead lock(impl->system->GetBodyLockInterface(), result.mBodyID);
    if (lock.Succeeded())
      hit.normal = to_glm(lock.GetBody().GetWorldSpaceSurfaceNormal(result.mSubShapeID2, point));
  }

  if (RigidBody* rigid_body = impl->find(result.mBodyID))
    hit.game_object = rigid_body->get_game_object();

  return hit;
}

}
