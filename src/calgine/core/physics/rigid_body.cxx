#include "physics_internal.h"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

#include "calgine/core/physics/rigid_body.h"
#include "calgine/core/game_object.h"
#include "calgine/core/event_context.h"
#include "calgine/core/transform.h"
#include "calgine/core/behaviour_serialization/behaviour_register_macro.h"

namespace Calgine {

namespace {

JPH::EMotionType to_jolt_motion(MotionType motion)
{
  switch (motion)
  {
    case MotionType::static_body: return JPH::EMotionType::Static;
    case MotionType::kinematic:   return JPH::EMotionType::Kinematic;
    case MotionType::dynamic:     return JPH::EMotionType::Dynamic;
  }
  return JPH::EMotionType::Dynamic;
}

JPH::RefConst<JPH::Shape> build_shape(const ColliderShape& shape, const glm::vec3& world_scale)
{
  const glm::vec3 scale = glm::abs(world_scale);

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
    case ColliderShape::box:
    default:
    {
      const glm::vec3 half_extents = glm::max(shape.half_extents * scale, glm::vec3(0.001f));
      // BoxShape clamps the convex radius to the smallest half extent itself.
      return new JPH::BoxShape(to_jolt(half_extents));
    }
  }
}

}

PhysicsWorld::Impl* RigidBody::world_impl()
{
  return PhysicsWorld::get_instance().get_impl();
}

RigidBody::RigidBody() = default;

RigidBody::RigidBody(const RigidBodySettings& _settings) : settings(_settings) {}

RigidBody::~RigidBody()
{
  destroy_body();
}

bool RigidBody::is_valid() const
{
  return body_id != invalid_body_id && world_impl() != nullptr;
}

void RigidBody::start_tick()
{
  create_body();
}

void RigidBody::on_destroy()
{
  destroy_body();
}

void RigidBody::create_body()
{
  PhysicsWorld::Impl* impl = world_impl();
  if (!impl)
  {
    Log::get_engine_logger()->error("RigidBody on '{}' started before the physics world exists (call App::start_systems() first).",
                                    get_game_object()->get_name());
    return;
  }

  if (body_id != invalid_body_id)
    return;

  GameObject* game_object = get_game_object();
  const Transform world = game_object->get_world_transform();

  const bool is_static = settings.motion == MotionType::static_body;

  JPH::BodyCreationSettings creation(
    build_shape(settings.shape, world.scale),
    to_jolt_r(world.position),
    to_jolt(world.get_rotation_quat()),
    to_jolt_motion(settings.motion),
    is_static ? PhysicsLayers::non_moving : PhysicsLayers::moving);

  creation.mFriction = settings.friction;
  creation.mRestitution = settings.restitution;
  creation.mLinearDamping = settings.linear_damping;
  creation.mAngularDamping = settings.angular_damping;
  creation.mGravityFactor = settings.gravity_factor;
  creation.mIsSensor = settings.is_sensor;
  // Let kinematic sensors detect static geometry too.
  creation.mCollideKinematicVsNonDynamic = settings.is_sensor;

  if (settings.mass > 0.0f && settings.motion == MotionType::dynamic)
  {
    creation.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
    creation.mMassPropertiesOverride.mMass = settings.mass;
  }

  JPH::BodyInterface& body_interface = impl->body_interface();

  const bool start_in_simulation = game_object->is_enabled();
  JPH::BodyID id;
  if (start_in_simulation)
  {
    id = body_interface.CreateAndAddBody(creation, is_static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);
  }
  else
  {
    JPH::Body* body = body_interface.CreateBody(creation);
    if (body)
      id = body->GetID();
  }

  if (id.IsInvalid())
  {
    Log::get_engine_logger()->error("Failed to create physics body for '{}' (body limit reached?).", game_object->get_name());
    return;
  }

  body_id = id.GetIndexAndSequenceNumber();
  in_simulation = start_in_simulation;
  impl->bodies[body_id] = this;
}

void RigidBody::destroy_body()
{
  if (body_id == invalid_body_id)
    return;

  PhysicsWorld::Impl* impl = world_impl();
  if (impl)
  {
    const JPH::BodyID id(body_id);
    JPH::BodyInterface& body_interface = impl->body_interface();

    impl->bodies.erase(body_id);
    if (body_interface.IsAdded(id))
      body_interface.RemoveBody(id);
    body_interface.DestroyBody(id);
  }

  body_id = invalid_body_id;
  in_simulation = false;
}

void RigidBody::pre_step(float delta_time)
{
  if (!is_valid())
    return;

  JPH::BodyInterface& body_interface = world_impl()->body_interface();
  const JPH::BodyID id(body_id);
  GameObject* game_object = get_game_object();

  // Follow the GameObject's enabled state.
  const bool should_simulate = game_object->is_enabled();
  if (should_simulate != in_simulation)
  {
    if (should_simulate)
    {
      body_interface.SetPositionAndRotation(id, to_jolt_r(game_object->get_world_position()),
                                            to_jolt(game_object->get_world_rotation()), JPH::EActivation::DontActivate);
      body_interface.AddBody(id, settings.motion == MotionType::static_body ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);
    }
    else
    {
      body_interface.RemoveBody(id);
    }
    in_simulation = should_simulate;
  }

  if (in_simulation && settings.motion == MotionType::kinematic)
  {
    // Sweep towards wherever the GameObject (or its parents) moved it.
    body_interface.MoveKinematic(id, to_jolt_r(game_object->get_world_position()),
                                 to_jolt(game_object->get_world_rotation()), delta_time);
  }
}

void RigidBody::post_step()
{
  if (!is_valid() || !in_simulation || settings.motion != MotionType::dynamic)
    return;

  JPH::BodyInterface& body_interface = world_impl()->body_interface();
  const JPH::BodyID id(body_id);

  if (!body_interface.IsActive(id))
    return; // asleep: transform hasn't changed

  JPH::RVec3 position;
  JPH::Quat rotation;
  body_interface.GetPositionAndRotation(id, position, rotation);

  GameObject* game_object = get_game_object();
  game_object->set_world_position(to_glm(position));
  game_object->set_world_rotation(to_glm(rotation));
}

void RigidBody::add_force(const glm::vec3& force)
{
  if (is_valid())
    world_impl()->body_interface().AddForce(JPH::BodyID(body_id), to_jolt(force));
}

void RigidBody::add_impulse(const glm::vec3& impulse)
{
  if (is_valid())
    world_impl()->body_interface().AddImpulse(JPH::BodyID(body_id), to_jolt(impulse));
}

void RigidBody::add_torque(const glm::vec3& torque)
{
  if (is_valid())
    world_impl()->body_interface().AddTorque(JPH::BodyID(body_id), to_jolt(torque));
}

void RigidBody::set_linear_velocity(const glm::vec3& velocity)
{
  if (is_valid())
    world_impl()->body_interface().SetLinearVelocity(JPH::BodyID(body_id), to_jolt(velocity));
}

glm::vec3 RigidBody::get_linear_velocity() const
{
  if (!is_valid())
    return glm::vec3(0.0f);
  return to_glm(world_impl()->body_interface().GetLinearVelocity(JPH::BodyID(body_id)));
}

void RigidBody::set_angular_velocity(const glm::vec3& velocity)
{
  if (is_valid())
    world_impl()->body_interface().SetAngularVelocity(JPH::BodyID(body_id), to_jolt(velocity));
}

glm::vec3 RigidBody::get_angular_velocity() const
{
  if (!is_valid())
    return glm::vec3(0.0f);
  return to_glm(world_impl()->body_interface().GetAngularVelocity(JPH::BodyID(body_id)));
}

void RigidBody::teleport(const glm::vec3& world_position, const glm::quat& world_rotation)
{
  GameObject* game_object = get_game_object();
  game_object->set_world_position(world_position);
  game_object->set_world_rotation(world_rotation);

  if (is_valid())
  {
    world_impl()->body_interface().SetPositionAndRotation(
      JPH::BodyID(body_id), to_jolt_r(world_position), to_jolt(world_rotation),
      settings.motion == MotionType::static_body ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);
  }
}

void RigidBody::teleport(const glm::vec3& world_position, const glm::vec3& world_rotation_degrees)
{
  teleport(world_position, Transform::euler_degrees_to_quat(world_rotation_degrees));
}

std::vector<const CollisionEvent*> RigidBody::get_collisions(const EventContext& event_context) const
{
  std::vector<const CollisionEvent*> result;
  const GameObject* self = const_cast<RigidBody*>(this)->get_game_object();

  for (const CollisionEvent* collision : event_context.get_events<CollisionEvent>())
  {
    if (collision->involves(self))
      result.push_back(collision);
  }
  return result;
}

CALGINE_REGISTER_BEHAVIOUR(RigidBody, "rigid_body");

}
