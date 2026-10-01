#include "physics_internal.h"

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

/** True if the world matrix is skewed (non-uniform scale under a rotated parent), which colliders can't represent. */
bool is_skewed(const glm::mat4& world)
{
  const glm::vec3 x = glm::normalize(glm::vec3(world[0]));
  const glm::vec3 y = glm::normalize(glm::vec3(world[1]));
  const glm::vec3 z = glm::normalize(glm::vec3(world[2]));
  constexpr float tolerance = 0.01f;
  return std::abs(glm::dot(x, y)) > tolerance || std::abs(glm::dot(y, z)) > tolerance || std::abs(glm::dot(x, z)) > tolerance;
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
  const std::string name = game_object->get_name();
  const glm::mat4 world_matrix = game_object->get_world_matrix();
  const Transform world = Transform::from_matrix(world_matrix);

  if (is_skewed(world_matrix))
  {
    Log::get_engine_logger()->warn("RigidBody on '{}': non-uniform scale under a rotated parent skews the object; "
                                   "its collider can't be skewed, so it only approximates the mesh.", name);
  }

  if (settings.layer >= max_collision_layers)
  {
    Log::get_engine_logger()->error("RigidBody on '{}': collision layer {} is out of range; using layer 0.", name, settings.layer);
    settings.layer = default_collision_layer;
  }

  JPH::RefConst<JPH::Shape> shape = impl->build_shape(settings.shape, world.scale, settings.motion, name);
  const bool is_static = settings.motion == MotionType::static_body;
  const glm::quat rotation = world.get_rotation_quat();

  JPH::BodyCreationSettings creation(
    shape,
    to_jolt_r(world.position),
    to_jolt(rotation),
    to_jolt_motion(settings.motion),
    make_object_layer(settings.layer, !is_static));

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
    Log::get_engine_logger()->error("Failed to create physics body for '{}' (body limit reached? see app.settings.physics.max_bodies).", name);
    return;
  }

  body_id = id.GetIndexAndSequenceNumber();

  BodyRecord& record = impl->bodies[body_id];
  record.owner = this;
  record.id = id;
  record.motion = settings.motion;
  record.in_simulation = start_in_simulation;
  record.is_sensor = settings.is_sensor;
  record.interpolate = settings.interpolate;
  record.track_scale = settings.track_scale;
  record.shape_scale = world.scale;
  record.shape = shape;
  record.previous = record.current = Pose { world.position, rotation };
}

void RigidBody::destroy_body()
{
  if (body_id == invalid_body_id)
    return;

  PhysicsWorld::Impl* impl = world_impl();
  if (impl)
  {
    // Joints can't outlive their bodies.
    impl->remove_joints_of_body(body_id);

    const JPH::BodyID id(body_id);
    JPH::BodyInterface& body_interface = impl->body_interface();

    impl->bodies.erase(body_id);
    if (body_interface.IsAdded(id))
      body_interface.RemoveBody(id);
    body_interface.DestroyBody(id);

    get_game_object()->clear_render_override();
  }

  body_id = invalid_body_id;
}

void RigidBody::refresh_shape()
{
  if (!is_valid())
    return;

  PhysicsWorld::Impl* impl = world_impl();
  BodyRecord* record = impl->record(body_id);
  if (!record)
    return;

  GameObject* game_object = get_game_object();
  const glm::vec3 world_scale = game_object->get_world_scale();

  record->shape = impl->build_shape(settings.shape, world_scale, settings.motion, game_object->get_name());
  record->shape_scale = world_scale;

  const bool update_mass = settings.motion == MotionType::dynamic && settings.mass <= 0.0f;
  impl->body_interface().SetShape(record->id, record->shape, update_mass,
                                  record->in_simulation && settings.motion != MotionType::static_body
                                    ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
}

void RigidBody::set_layer(CollisionLayer layer)
{
  if (layer >= max_collision_layers)
  {
    Log::get_engine_logger()->error("RigidBody::set_layer: layer {} is out of range.", layer);
    return;
  }

  settings.layer = layer;
  if (is_valid())
    world_impl()->body_interface().SetObjectLayer(JPH::BodyID(body_id), make_object_layer(layer, settings.motion != MotionType::static_body));
}

void RigidBody::set_interpolate(bool interpolate)
{
  settings.interpolate = interpolate;
  if (is_valid())
    if (BodyRecord* record = world_impl()->record(body_id))
      record->interpolate = interpolate;
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

  if (!is_valid())
    return;

  PhysicsWorld::Impl* impl = world_impl();
  impl->body_interface().SetPositionAndRotation(
    JPH::BodyID(body_id), to_jolt_r(world_position), to_jolt(world_rotation),
    settings.motion == MotionType::static_body ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);

  if (BodyRecord* record = impl->record(body_id))
  {
    // Jump straight there: no interpolating across the teleport, and it doesn't count as a stray kinematic move.
    const Pose pose { world_position, glm::normalize(world_rotation) };
    record->previous = record->current = pose;
    record->last_fixed_logical = pose;
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
