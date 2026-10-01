#include "physics_internal.h"

#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include "calgine/core/physics/rigid_body.h"
#include "calgine/core/physics/collision_event.h"
#include "calgine/core/event_context.h"
#include "calgine/core/game_object.h"
#include "calgine/core/renderer/model.h"
#include "calgine/core/renderer/renderer.h"
#include "calgine/core/renderer/camera/camera_manager.h"
#include "calgine/core/log.h"

#include <cstdarg>
#include <cstring>
#include <thread>

namespace Calgine {

namespace {

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

Pose logical_world_pose(const GameObject* game_object)
{
  return Pose { game_object->get_world_position(), game_object->get_world_rotation() };
}

Pose blend(const Pose& from, const Pose& to, float alpha)
{
  return Pose { glm::mix(from.position, to.position, alpha), glm::slerp(from.rotation, to.rotation, alpha) };
}

bool scale_changed(const glm::vec3& a, const glm::vec3& b)
{
  return glm::any(glm::greaterThan(glm::abs(a - b), glm::vec3(1e-4f)));
}

/** Hash for welding identical vertex positions together. */
struct PositionHash
{
  size_t operator()(const glm::vec3& v) const
  {
    uint32_t bits[3];
    std::memcpy(bits, &v, sizeof(bits));
    size_t hash = bits[0];
    hash = hash * 31 + bits[1];
    hash = hash * 31 + bits[2];
    return hash;
  }
};

/**
 * World-space info about a parent GameObject, computed once per step and shared by all its children.
 * Most scenes put many bodies under one parent, so this turns thousands of matrix inversions into one.
 */
struct ParentSpace
{
  glm::mat4 inverse_world = glm::mat4(1.0f);
  glm::quat inverse_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 world_scale = glm::vec3(1.0f);
};

ParentSpace make_parent_space(const GameObject& parent)
{
  const glm::mat4 world = parent.get_world_matrix();

  ParentSpace space;
  space.inverse_world = glm::inverse(world);

  glm::mat3 basis(world);
  space.world_scale = glm::vec3(glm::length(basis[0]), glm::length(basis[1]), glm::length(basis[2]));
  for (int i = 0; i < 3; i++)
    if (space.world_scale[i] > 0.0f)
      basis[i] /= space.world_scale[i];
  space.inverse_rotation = glm::conjugate(glm::normalize(glm::quat_cast(basis)));
  return space;
}

/** Looks up (or computes) parent spaces, remembering the last parent since siblings are usually processed together. */
class ParentSpaceCache
{
public:
  const ParentSpace* get(const GameObject* game_object)
  {
    std::optional<std::reference_wrapper<GameObject>> parent = game_object->get_parent();
    if (!parent)
      return nullptr;

    const GameObject* parent_ptr = &parent->get();
    if (parent_ptr == last_parent)
      return last_space;

    auto [it, inserted] = spaces.try_emplace(parent_ptr);
    if (inserted)
      it->second = make_parent_space(*parent_ptr);

    last_parent = parent_ptr;
    last_space = &it->second;
    return last_space;
  }

  void clear()
  {
    spaces.clear();
    last_parent = nullptr;
    last_space = nullptr;
  }

private:
  std::unordered_map<const GameObject*, ParentSpace> spaces;
  const GameObject* last_parent = nullptr;
  const ParentSpace* last_space = nullptr;
};

int hierarchy_depth(const GameObject* game_object)
{
  int depth = 0;
  std::optional<std::reference_wrapper<GameObject>> parent = game_object->get_parent();
  while (parent)
  {
    depth++;
    parent = parent->get().get_parent();
  }
  return depth;
}

/** Only bodies in @p mask pass. */
class LayerMaskFilter final : public JPH::ObjectLayerFilter
{
public:
  explicit LayerMaskFilter(CollisionLayerMask _mask) : mask(_mask) {}
  bool ShouldCollide(JPH::ObjectLayer layer) const override
  {
    const CollisionLayer collision_layer = collision_layer_of(layer);
    return collision_layer < max_collision_layers && (mask & collision_layer_bit(collision_layer)) != 0;
  }

private:
  CollisionLayerMask mask;
};

class SensorFilter final : public JPH::BodyFilter
{
public:
  explicit SensorFilter(bool _include_sensors) : include_sensors(_include_sensors) {}
  bool ShouldCollideLocked(const JPH::Body& body) const override { return include_sensors || !body.IsSensor(); }

private:
  bool include_sensors;
};

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

// ---- Shapes ----

JPH::RefConst<JPH::Shape> PhysicsWorld::Impl::get_model_shape(const std::shared_ptr<Model>& model, bool triangle_mesh, const std::string& owner_name)
{
  ModelShapes& cache = model_shapes[model.get()];
  if (cache.model.expired() || cache.model.lock() != model)
  {
    // A different model now lives at this address (or this is the first use): start fresh.
    cache = ModelShapes {};
    cache.model = model;
  }

  JPH::RefConst<JPH::Shape>& cached = triangle_mesh ? cache.mesh : cache.convex_hull;
  if (cached)
    return cached;

  const std::vector<glm::vec3>& positions = model->mesh.get_positions();
  const std::vector<uint32_t>& indices = model->mesh.get_indices();
  if (positions.empty())
  {
    Log::get_engine_logger()->error("Collider for '{}': model has no vertices.", owner_name);
    return nullptr;
  }

  // Models are loaded with one vertex per triangle corner. Weld identical positions so the collision
  // shape shares edges (smoother sliding over triangle meshes, fewer points for hulls).
  std::unordered_map<glm::vec3, uint32_t, PositionHash> welded_index;
  std::vector<glm::vec3> welded;
  std::vector<uint32_t> remap(positions.size());
  for (size_t i = 0; i < positions.size(); i++)
  {
    auto [it, inserted] = welded_index.try_emplace(positions[i], static_cast<uint32_t>(welded.size()));
    if (inserted)
      welded.push_back(positions[i]);
    remap[i] = it->second;
  }

  JPH::Shape::ShapeResult result;
  if (triangle_mesh)
  {
    JPH::VertexList vertices;
    vertices.reserve(welded.size());
    for (const glm::vec3& p : welded)
      vertices.push_back(JPH::Float3(p.x, p.y, p.z));

    JPH::IndexedTriangleList triangles;
    triangles.reserve(indices.size() / 3);
    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
      const uint32_t a = remap[indices[i]], b = remap[indices[i + 1]], c = remap[indices[i + 2]];
      if (a != b && b != c && c != a)
        triangles.push_back(JPH::IndexedTriangle(a, b, c, 0));
    }

    JPH::MeshShapeSettings settings(std::move(vertices), std::move(triangles));
    result = settings.Create();
  }
  else
  {
    JPH::Array<JPH::Vec3> points;
    points.reserve(welded.size());
    for (const glm::vec3& p : welded)
      points.push_back(to_jolt(p));

    JPH::ConvexHullShapeSettings settings(points, JPH::cDefaultConvexRadius);
    result = settings.Create();
  }

  if (result.HasError())
  {
    Log::get_engine_logger()->error("Collider for '{}': building {} failed: {}", owner_name,
                                    triangle_mesh ? "triangle mesh" : "convex hull", result.GetError().c_str());
    return nullptr;
  }

  cached = result.Get();
  return cached;
}

JPH::RefConst<JPH::Shape> PhysicsWorld::Impl::build_shape(const ColliderShape& shape, const glm::vec3& world_scale, MotionType motion, const std::string& owner_name)
{
  const glm::vec3 scale = glm::max(glm::abs(world_scale), glm::vec3(0.001f));

  switch (shape.type)
  {
    case ColliderShape::sphere:
    {
      const float max_scale = std::max(scale.x, std::max(scale.y, scale.z));
      return new JPH::SphereShape(std::max(shape.radius * max_scale, 0.001f));
    }
    case ColliderShape::capsule:
    {
      const float radius_scale = std::max(scale.x, scale.z);
      return new JPH::CapsuleShape(std::max(shape.half_height * scale.y, 0.001f),
                                   std::max(shape.radius * radius_scale, 0.001f));
    }
    case ColliderShape::convex_hull:
    case ColliderShape::mesh:
    {
      if (!shape.model)
      {
        Log::get_engine_logger()->error("Collider for '{}': convex_hull/mesh collider has no model; using a box.", owner_name);
        break;
      }

      bool triangle_mesh = shape.type == ColliderShape::mesh;
      if (triangle_mesh && motion == MotionType::dynamic)
      {
        Log::get_engine_logger()->warn("Collider for '{}': triangle mesh colliders can't be dynamic (Jolt restriction); using a convex hull instead.", owner_name);
        triangle_mesh = false;
      }

      JPH::RefConst<JPH::Shape> base = get_model_shape(shape.model, triangle_mesh, owner_name);
      if (!base)
        break;

      if (!scale_changed(scale, glm::vec3(1.0f)))
        return base;
      return new JPH::ScaledShape(base, to_jolt(scale));
    }
    case ColliderShape::box:
    default:
      break;
  }

  // Box (also the fallback for failed model shapes).
  const glm::vec3 half_extents = glm::max(shape.half_extents * scale, glm::vec3(0.001f));
  // BoxShape clamps the convex radius to the smallest half extent itself.
  return new JPH::BoxShape(to_jolt(half_extents));
}

Pose PhysicsWorld::Impl::read_body_pose(const JPH::BodyID& id)
{
  JPH::RVec3 position;
  JPH::Quat rotation;
  system->GetBodyInterfaceNoLock().GetPositionAndRotation(id, position, rotation);
  return Pose { to_glm(position), to_glm(rotation) };
}

// ---- PhysicsWorld ----

PhysicsWorld::PhysicsWorld() = default;
PhysicsWorld::~PhysicsWorld() = default;

PhysicsWorld& PhysicsWorld::get_instance()
{
  static PhysicsWorld instance;
  return instance;
}

void PhysicsWorld::init(const PhysicsSettings& _settings)
{
  if (impl)
  {
    Log::get_engine_logger()->warn("PhysicsWorld::init() called twice, ignoring.");
    return;
  }

  settings = _settings;

  Log::get_engine_logger()->info("Initializing Jolt Physics");

  JPH::RegisterDefaultAllocator();

  JPH::Trace = jolt_trace;
  JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = jolt_assert_failed;)

  JPH::Factory::sInstance = new JPH::Factory();
  JPH::RegisterTypes();

  impl = std::make_unique<Impl>();

  const JPH::uint temp_bytes = std::max<JPH::uint>(1, settings.temp_allocator_mb) * 1024u * 1024u;
  impl->temp_allocator = std::make_unique<JPH::TempAllocatorImplWithMallocFallback>(temp_bytes);

  const int worker_threads = settings.worker_threads >= 0
    ? settings.worker_threads
    : std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
  impl->job_system = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, worker_threads);

  // The pair filter reads the live layer table, so layer changes at runtime take effect on the next step.
  impl->object_pair_filter.layers = &settings.layers;

  impl->system = std::make_unique<JPH::PhysicsSystem>();
  impl->system->Init(settings.max_bodies, 0, settings.max_body_pairs, settings.max_contact_constraints,
                     impl->broad_phase_layer_interface, impl->object_vs_broad_phase_filter, impl->object_pair_filter);
  impl->system->SetContactListener(&impl->contact_queue);
  impl->system->SetGravity(to_jolt(settings.gravity));

#ifdef JPH_DEBUG_RENDERER
  impl->debug_renderer = std::make_unique<PhysicsDebugRenderer>();
#endif

  Log::get_engine_logger()->info("Jolt Physics ready ({} worker threads, up to {} bodies)", worker_threads, settings.max_bodies);
}

void PhysicsWorld::shutdown()
{
  if (!impl)
    return;

  JPH::BodyInterface& body_interface = impl->body_interface();

  // Joints first: Jolt constraints must not outlive their bodies.
  for (auto& [id, record] : impl->joints)
  {
    impl->system->RemoveConstraint(record.constraint);
    if (record.owner)
      record.owner->joint_id = Joint::invalid_joint_id;
  }
  impl->joints.clear();
  impl->pending_joints.clear();

  // Remove and destroy every body, and detach RigidBodies so their destructors don't touch the world later
  // (the static hierarchies are destroyed after the App).
  for (auto& [id, record] : impl->bodies)
  {
    if (body_interface.IsAdded(record.id))
      body_interface.RemoveBody(record.id);
    body_interface.DestroyBody(record.id);

    record.owner->body_id = RigidBody::invalid_body_id;
  }
  impl->bodies.clear();
  impl->model_shapes.clear();
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

void PhysicsWorld::begin_fixed_phase()
{
  if (!impl || !settings.warn_kinematic_moved_outside_fixed_update)
    return;

  // Kinematic bodies should only move during fixed update. If one has moved since the last fixed phase
  // ended, something moved it in update/late_update/render, and its collider will trail its mesh.
  for (auto& [id, record] : impl->bodies)
  {
    if (record.motion != MotionType::kinematic || !record.in_simulation || !record.has_last_fixed_logical || record.warned_moved_outside_fixed)
      continue;

    const GameObject* game_object = record.owner->get_game_object();
    const Pose now = logical_world_pose(game_object);
    const bool moved = glm::length(now.position - record.last_fixed_logical.position) > 1e-4f
                    || std::abs(glm::dot(now.rotation, record.last_fixed_logical.rotation)) < 0.999999f;
    if (moved)
    {
      record.warned_moved_outside_fixed = true;
      Log::get_engine_logger()->warn(
        "Kinematic body '{}' (or a parent) was moved outside fixed_update_tick(). Its collider will trail its mesh by a frame; "
        "move it in fixed_update_tick() using Time::fixed_delta_time(). (Warned once per body.)",
        const_cast<GameObject*>(game_object)->get_name());
    }
  }
}

void PhysicsWorld::step(float delta_time, EventContext& event_context)
{
  if (!impl)
    return;

  using Clock = std::chrono::steady_clock;
  const auto to_ms = [](Clock::duration duration) { return std::chrono::duration<float, std::milli>(duration).count(); };

  PhysicsStepStats stats;
  const Clock::time_point step_start = Clock::now();

  // ---- Pre-step ----

  // Create joints whose bodies now exist.
  std::erase_if(impl->pending_joints, [this](Joint* joint) { return impl->try_create_joint(joint); });

  JPH::BodyInterface& body_interface = impl->body_interface();
  for (auto& [id, record] : impl->bodies)
  {
    GameObject* game_object = record.owner->get_game_object();

    // Follow the GameObject's enabled state.
    const bool should_simulate = game_object->is_enabled();
    if (should_simulate != record.in_simulation)
    {
      if (should_simulate)
      {
        const Pose pose = logical_world_pose(game_object);
        body_interface.SetPositionAndRotation(record.id, to_jolt_r(pose.position), to_jolt(pose.rotation), JPH::EActivation::DontActivate);
        body_interface.AddBody(record.id, record.motion == MotionType::static_body ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);
        impl->bodies_added_since_step++;
        record.previous = record.current = pose;
      }
      else
      {
        body_interface.RemoveBody(record.id);
        game_object->clear_render_override();
      }
      record.in_simulation = should_simulate;
      impl->refresh_joints_of_body(id);
    }

    if (!record.in_simulation)
      continue;

    if (record.track_scale)
    {
      const glm::vec3 world_scale = game_object->get_world_scale();
      if (scale_changed(world_scale, record.shape_scale))
        record.owner->refresh_shape();
    }

    if (record.motion == MotionType::kinematic)
    {
      // Sweep towards wherever the GameObject (or its parents) moved it.
      const Pose target = logical_world_pose(game_object);
      body_interface.MoveKinematic(record.id, to_jolt_r(target.position), to_jolt(target.rotation), delta_time);
    }
  }

  // ---- Simulate ----

  const Clock::time_point simulate_start = Clock::now();

  // Bodies added one at a time leave the broad phase tree unbalanced, which slows every query until Jolt
  // slowly rebuilds it. After a big batch, rebuild it now. Small spawns are left to Jolt's incremental rebuild.
  constexpr uint32_t optimize_broad_phase_threshold = 256;
  if (impl->bodies_added_since_step >= optimize_broad_phase_threshold)
    impl->system->OptimizeBroadPhase();
  impl->bodies_added_since_step = 0;

  JPH::EPhysicsUpdateError error = impl->system->Update(delta_time, std::max(1, settings.collision_steps),
                                                        impl->temp_allocator.get(), impl->job_system.get());
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
      Log::get_engine_logger()->warn("Jolt dropped some contacts this step:{} Raise the limits in app.settings.physics.", reasons);

      impl->last_logged_error = flags;
      impl->last_error_log_time = now;
    }
  }

  // ---- Post-step: record poses and write dynamic bodies back ----

  const Clock::time_point post_start = Clock::now();

  const JPH::BodyInterface& no_lock = impl->system->GetBodyInterfaceNoLock();

  std::vector<std::pair<int, BodyRecord*>> write_backs;
  write_backs.reserve(impl->bodies.size());

  for (auto& [id, record] : impl->bodies)
  {
    if (!record.in_simulation || record.motion == MotionType::static_body)
      continue;

    record.previous = record.current;
    if (!no_lock.IsActive(record.id))
      continue; // asleep: pose unchanged

    record.current = impl->read_body_pose(record.id);
    if (record.motion == MotionType::dynamic)
      write_backs.emplace_back(0, &record);
  }

  // Parents must be written before their children, or a child's local transform would be computed
  // against its parent's old pose. Sort by hierarchy depth (usually every body has the same depth).
  bool mixed_depths = false;
  for (auto& [depth, record] : write_backs)
  {
    depth = hierarchy_depth(record->owner->get_game_object());
    mixed_depths = mixed_depths || depth != write_backs.front().first;
  }
  if (mixed_depths)
    std::stable_sort(write_backs.begin(), write_backs.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

  ParentSpaceCache parent_spaces;
  int current_depth = write_backs.empty() ? 0 : write_backs.front().first;
  for (auto& [depth, record] : write_backs)
  {
    if (depth != current_depth)
    {
      // Parents at the previous depth may just have moved.
      parent_spaces.clear();
      current_depth = depth;
    }

    GameObject* game_object = record->owner->get_game_object();
    Transform& local = game_object->get_transform();

    if (const ParentSpace* parent = parent_spaces.get(game_object))
    {
      local.position = glm::vec3(parent->inverse_world * glm::vec4(record->current.position, 1.0f));
      local.set_rotation_quat(parent->inverse_rotation * record->current.rotation);
    }
    else
    {
      local.position = record->current.position;
      local.set_rotation_quat(record->current.rotation);
    }
  }

  // ---- Events ----

  const Clock::time_point events_start = Clock::now();

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
  stats.joints = static_cast<uint32_t>(impl->joints.size());

  last_step_stats = stats;
  total_steps++;

  if (step_stats_callback)
    step_stats_callback(stats);
}

void PhysicsWorld::update_interpolation(float alpha)
{
  if (!impl)
    return;

  const auto start = std::chrono::steady_clock::now();

  alpha = std::clamp(alpha, 0.0f, 1.0f);
  impl->last_alpha = alpha;

  ParentSpaceCache parent_spaces;
  for (auto& [id, record] : impl->bodies)
  {
    GameObject* game_object = record.owner->get_game_object();

    // Remember where kinematic bodies were when the fixed phase ended (see begin_fixed_phase()).
    if (record.motion == MotionType::kinematic && record.in_simulation && settings.warn_kinematic_moved_outside_fixed_update)
    {
      record.last_fixed_logical = logical_world_pose(game_object);
      record.has_last_fixed_logical = true;
    }

    const bool interpolate = settings.interpolation && record.interpolate && record.in_simulation && record.motion != MotionType::static_body;
    if (!interpolate)
    {
      if (game_object->has_render_override())
        game_object->clear_render_override();
      continue;
    }

    // Draw with the object's current scale (parent scale x local scale), so scaling a body still shows
    // even if its collider hasn't been refreshed.
    glm::vec3 scale = game_object->get_transform().scale;
    if (const ParentSpace* parent = parent_spaces.get(game_object))
      scale *= parent->world_scale;

    const Pose pose = blend(record.previous, record.current, alpha);
    glm::mat4 matrix = glm::mat4_cast(pose.rotation);
    matrix[0] *= scale.x;
    matrix[1] *= scale.y;
    matrix[2] *= scale.z;
    matrix[3] = glm::vec4(pose.position, 1.0f);
    game_object->set_render_override(matrix);
  }

  last_interpolation_ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - start).count();
}

void PhysicsWorld::debug_draw()
{
#ifdef JPH_DEBUG_RENDERER
  if (!impl || !debug_draw_enabled)
    return;

  if (CameraBehaviour* camera = CameraManager::get_instance().get_active_camera())
    impl->debug_renderer->SetCameraPos(to_jolt_r(camera->get_game_object()->get_render_transform().position));

  const JPH::BodyInterface& no_lock = impl->system->GetBodyInterfaceNoLock();

  // Draw each collider where its mesh is drawn (the interpolated pose), so wireframes line up exactly.
  for (auto& [id, record] : impl->bodies)
  {
    if (!record.in_simulation || !record.shape)
      continue;

    const bool interpolated = settings.interpolation && record.interpolate && record.motion != MotionType::static_body;
    const Pose pose = interpolated ? blend(record.previous, record.current, impl->last_alpha) : impl->read_body_pose(record.id);

    JPH::Color colour = JPH::Color::sGrey;
    if (record.is_sensor)
      colour = JPH::Color::sCyan;
    else if (record.motion == MotionType::kinematic)
      colour = JPH::Color::sGreen;
    else if (record.motion == MotionType::dynamic)
      colour = no_lock.IsActive(record.id) ? JPH::Color::sOrange : JPH::Color(80, 110, 200);

    const JPH::RMat44 world = JPH::RMat44::sRotationTranslation(to_jolt(pose.rotation), to_jolt_r(pose.position));
    record.shape->Draw(impl->debug_renderer.get(), world.PreTranslated(record.shape->GetCenterOfMass()),
                       JPH::Vec3::sReplicate(1.0f), colour, false, true);
  }

  impl->system->DrawConstraints(impl->debug_renderer.get());
  impl->debug_renderer->NextFrame();
#endif
}

void PhysicsWorld::set_gravity(const glm::vec3& gravity)
{
  settings.gravity = gravity;
  if (impl)
    impl->system->SetGravity(to_jolt(gravity));
}

glm::vec3 PhysicsWorld::get_gravity() const
{
  return settings.gravity;
}

void PhysicsWorld::set_layers_collide(CollisionLayer a, CollisionLayer b, bool collide)
{
  settings.layers.set_collides(a, b, collide);
}

const std::string& PhysicsWorld::get_layer_name(CollisionLayer layer) const
{
  static const std::string empty;
  return layer < max_collision_layers ? settings.layers.names[layer] : empty;
}

size_t PhysicsWorld::get_body_count() const
{
  return impl ? impl->bodies.size() : 0;
}

size_t PhysicsWorld::get_joint_count() const
{
  return impl ? impl->joints.size() : 0;
}

std::optional<RaycastHit> PhysicsWorld::raycast(const glm::vec3& origin, const glm::vec3& direction, float max_distance,
                                                CollisionLayerMask layers, bool include_sensors) const
{
  if (!impl || max_distance <= 0.0f || glm::length(direction) <= 0.0f)
    return std::nullopt;

  const glm::vec3 ray_vector = glm::normalize(direction) * max_distance;
  const JPH::RRayCast ray(to_jolt_r(origin), to_jolt(ray_vector));

  const LayerMaskFilter layer_filter(layers);
  const SensorFilter sensor_filter(include_sensors);

  JPH::RayCastResult result;
  if (!impl->system->GetNarrowPhaseQuery().CastRay(ray, result, JPH::BroadPhaseLayerFilter(), layer_filter, sensor_filter))
    return std::nullopt;

  RaycastHit hit;
  hit.distance = result.mFraction * max_distance;
  const JPH::RVec3 point = ray.GetPointOnRay(result.mFraction);
  hit.point = to_glm(point);

  {
    JPH::BodyLockRead lock(impl->system->GetBodyLockInterface(), result.mBodyID);
    if (lock.Succeeded())
    {
      hit.normal = to_glm(lock.GetBody().GetWorldSpaceSurfaceNormal(result.mSubShapeID2, point));
      hit.layer = collision_layer_of(lock.GetBody().GetObjectLayer());
    }
  }

  if (RigidBody* rigid_body = impl->find(result.mBodyID))
    hit.game_object = rigid_body->get_game_object();

  return hit;
}

}
