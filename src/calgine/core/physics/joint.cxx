#include "physics_internal.h"

#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Constraints/PointConstraint.h>
#include <Jolt/Physics/Constraints/SliderConstraint.h>

#include "calgine/core/physics/joint.h"
#include "calgine/core/physics/rigid_body.h"
#include "calgine/core/game_object.h"
#include "calgine/core/log.h"
#include "calgine/core/behaviour_serialization/behaviour_register_macro.h"

namespace Calgine {

PhysicsWorld::Impl* Joint::world_impl()
{
  return PhysicsWorld::get_instance().get_impl();
}

// ---- World-side joint management ----

bool PhysicsWorld::Impl::try_create_joint(Joint* joint)
{
  if (joint->broken)
    return true; // drop: a broken joint never comes back

  GameObject* game_object = joint->get_game_object();
  const JointSettings& s = joint->settings;

  RigidBody* own = game_object->get_behaviour<RigidBody>();
  if (!own)
  {
    Log::get_engine_logger()->error("Joint on '{}' needs a RigidBody on the same GameObject.", game_object->get_name());
    return true;
  }

  RigidBody* other = s.connected_body;
  if (other == own)
  {
    Log::get_engine_logger()->error("Joint on '{}' is connected to its own body.", game_object->get_name());
    return true;
  }

  // Wait until both bodies exist.
  if (own->body_id == RigidBody::invalid_body_id || !record(own->body_id))
    return false;
  if (other && (other->body_id == RigidBody::invalid_body_id || !record(other->body_id)))
    return false;

  // World-space anchor and axis.
  const glm::vec3 anchor = s.anchor_is_local ? glm::vec3(game_object->get_world_matrix() * glm::vec4(s.anchor, 1.0f)) : s.anchor;
  glm::vec3 axis = s.axis_is_local ? game_object->get_world_rotation() * s.axis : s.axis;
  axis = glm::length(axis) > 0.0f ? glm::normalize(axis) : glm::vec3(0.0f, 1.0f, 0.0f);
  const JPH::Vec3 jolt_axis = to_jolt(axis);
  const JPH::Vec3 jolt_normal = jolt_axis.GetNormalizedPerpendicular();

  JPH::Ref<JPH::TwoBodyConstraintSettings> constraint_settings;
  switch (s.type)
  {
    case JointType::fixed:
    {
      auto* fixed = new JPH::FixedConstraintSettings();
      fixed->mAutoDetectPoint = true; // lock the current relative pose
      constraint_settings = fixed;
      break;
    }
    case JointType::point:
    {
      auto* point = new JPH::PointConstraintSettings();
      point->mPoint1 = point->mPoint2 = to_jolt_r(anchor);
      constraint_settings = point;
      break;
    }
    case JointType::hinge:
    {
      auto* hinge = new JPH::HingeConstraintSettings();
      hinge->mPoint1 = hinge->mPoint2 = to_jolt_r(anchor);
      hinge->mHingeAxis1 = hinge->mHingeAxis2 = jolt_axis;
      hinge->mNormalAxis1 = hinge->mNormalAxis2 = jolt_normal;
      if (s.limits_enabled)
      {
        // Jolt wants min in [-180, 0] and max in [0, 180] degrees.
        hinge->mLimitsMin = glm::radians(std::clamp(s.limit_min, -180.0f, 0.0f));
        hinge->mLimitsMax = glm::radians(std::clamp(s.limit_max, 0.0f, 180.0f));
      }
      hinge->mMotorSettings.SetTorqueLimit(s.motor_max_force);
      constraint_settings = hinge;
      break;
    }
    case JointType::slider:
    {
      auto* slider = new JPH::SliderConstraintSettings();
      slider->mAutoDetectPoint = true; // current relative position is "0"
      slider->SetSliderAxis(jolt_axis);
      if (s.limits_enabled)
      {
        slider->mLimitsMin = std::min(s.limit_min, 0.0f);
        slider->mLimitsMax = std::max(s.limit_max, 0.0f);
      }
      slider->mMotorSettings.SetForceLimit(s.motor_max_force);
      constraint_settings = slider;
      break;
    }
    case JointType::distance:
    {
      auto* distance = new JPH::DistanceConstraintSettings();
      distance->mPoint1 = to_jolt_r(anchor);
      const glm::vec3 other_anchor = other
        ? glm::vec3(other->get_game_object()->get_world_matrix() * glm::vec4(s.connected_anchor, 1.0f))
        : s.connected_anchor;
      distance->mPoint2 = to_jolt_r(other_anchor);
      distance->mMinDistance = s.min_distance;
      distance->mMaxDistance = s.max_distance;
      constraint_settings = distance;
      break;
    }
  }

  const JPH::BodyID other_id = other ? JPH::BodyID(other->body_id) : JPH::BodyID(); // invalid ID = attached to the world
  JPH::TwoBodyConstraint* constraint = body_interface().CreateConstraint(constraint_settings, JPH::BodyID(own->body_id), other_id);
  if (!constraint)
  {
    Log::get_engine_logger()->error("Failed to create joint on '{}'.", game_object->get_name());
    return true;
  }

  system->AddConstraint(constraint);

  const uint32_t id = next_joint_id++;
  JointRecord& joint_record = joints[id];
  joint_record.owner = joint;
  joint_record.constraint = constraint;
  joint_record.body_a = own->body_id;
  joint_record.body_b = other ? other->body_id : invalid_id;

  joint->joint_id = id;
  update_joint_enabled(joint_record);
  joint->apply_motor();
  return true;
}

void PhysicsWorld::Impl::update_joint_enabled(JointRecord& joint_record)
{
  BodyRecord* a = record(joint_record.body_a);
  BodyRecord* b = joint_record.body_b != invalid_id ? record(joint_record.body_b) : nullptr;

  const bool bodies_simulating = a && a->in_simulation && (joint_record.body_b == invalid_id || (b && b->in_simulation));
  const bool enabled = joint_record.owner && joint_record.owner->enabled && bodies_simulating;
  joint_record.constraint->SetEnabled(enabled);

  if (enabled)
  {
    // Wake the bodies so the joint takes effect immediately.
    for (BodyRecord* body : { a, b })
      if (body && body->motion != MotionType::static_body)
        body_interface().ActivateBody(body->id);
  }
}

void PhysicsWorld::Impl::refresh_joints_of_body(uint32_t body_id)
{
  for (auto& [id, joint_record] : joints)
    if (joint_record.body_a == body_id || joint_record.body_b == body_id)
      update_joint_enabled(joint_record);
}

void PhysicsWorld::Impl::remove_joint(uint32_t joint_id)
{
  auto it = joints.find(joint_id);
  if (it == joints.end())
    return;

  system->RemoveConstraint(it->second.constraint);
  if (it->second.owner)
    it->second.owner->joint_id = Joint::invalid_joint_id;
  joints.erase(it);
}

void PhysicsWorld::Impl::remove_joints_of_body(uint32_t body_id)
{
  std::vector<uint32_t> to_remove;
  for (auto& [id, joint_record] : joints)
    if (joint_record.body_a == body_id || joint_record.body_b == body_id)
      to_remove.push_back(id);

  for (uint32_t id : to_remove)
  {
    JointRecord& joint_record = joints[id];
    if (Joint* owner = joint_record.owner)
    {
      owner->broken = true;
      if (joint_record.body_b == body_id)
        owner->settings.connected_body = nullptr; // don't leave a dangling pointer
      Log::get_engine_logger()->info("Joint on '{}' broke because one of its bodies was destroyed.", owner->get_game_object()->get_name());
    }
    remove_joint(id);
  }
}

// ---- Joint behaviour ----

Joint::Joint() = default;

Joint::Joint(const JointSettings& _settings) : settings(_settings) {}

Joint::~Joint()
{
  remove_from_world();
}

void Joint::start_tick()
{
  PhysicsWorld::Impl* impl = world_impl();
  if (!impl)
  {
    Log::get_engine_logger()->error("Joint on '{}' started before the physics world exists.", get_game_object()->get_name());
    return;
  }

  // Created at the start of the next physics step, once both bodies exist.
  impl->pending_joints.push_back(this);
}

void Joint::on_destroy()
{
  remove_from_world();
}

void Joint::remove_from_world()
{
  PhysicsWorld::Impl* impl = world_impl();
  if (!impl)
  {
    joint_id = invalid_joint_id;
    return;
  }

  std::erase(impl->pending_joints, this);
  if (joint_id != invalid_joint_id)
    impl->remove_joint(joint_id);
  joint_id = invalid_joint_id;
}

bool Joint::is_active() const
{
  return joint_id != invalid_joint_id && world_impl() != nullptr;
}

void Joint::set_enabled(bool _enabled)
{
  enabled = _enabled;
  if (!is_active())
    return;

  PhysicsWorld::Impl* impl = world_impl();
  impl->update_joint_enabled(impl->joints[joint_id]);
}

void Joint::set_motor_enabled(bool motor_enabled)
{
  settings.motor_enabled = motor_enabled;
  apply_motor();
}

void Joint::set_motor_target_velocity(float velocity)
{
  settings.motor_target_velocity = velocity;
  apply_motor();
}

void Joint::apply_motor()
{
  if (!is_active())
    return;

  PhysicsWorld::Impl* impl = world_impl();
  JointRecord& joint_record = impl->joints[joint_id];
  const JPH::EMotorState state = settings.motor_enabled ? JPH::EMotorState::Velocity : JPH::EMotorState::Off;

  if (settings.type == JointType::hinge)
  {
    auto* hinge = static_cast<JPH::HingeConstraint*>(joint_record.constraint.GetPtr());
    hinge->GetMotorSettings().SetTorqueLimit(settings.motor_max_force);
    hinge->SetTargetAngularVelocity(glm::radians(settings.motor_target_velocity));
    hinge->SetMotorState(state);
  }
  else if (settings.type == JointType::slider)
  {
    auto* slider = static_cast<JPH::SliderConstraint*>(joint_record.constraint.GetPtr());
    slider->GetMotorSettings().SetForceLimit(settings.motor_max_force);
    slider->SetTargetVelocity(settings.motor_target_velocity);
    slider->SetMotorState(state);
  }
  else
  {
    return;
  }

  // A sleeping body won't notice a motor change until something wakes it.
  impl->update_joint_enabled(joint_record);
}

float Joint::get_current_value() const
{
  if (!is_active())
    return 0.0f;

  const JointRecord& joint_record = world_impl()->joints[joint_id];
  if (settings.type == JointType::hinge)
    return glm::degrees(static_cast<const JPH::HingeConstraint*>(joint_record.constraint.GetPtr())->GetCurrentAngle());
  if (settings.type == JointType::slider)
    return static_cast<const JPH::SliderConstraint*>(joint_record.constraint.GetPtr())->GetCurrentPosition();
  return 0.0f;
}

CALGINE_REGISTER_BEHAVIOUR(Joint, "joint");

}
