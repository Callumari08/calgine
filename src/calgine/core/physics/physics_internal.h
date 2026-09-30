#pragma once

// INTERNAL: included only by Calgine's physics .cxx files. Never include this from a public header,
// so games using Calgine don't need Jolt headers or Jolt's compile definitions.

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/PhysicsSystem.h>

#ifdef JPH_DEBUG_RENDERER
#include <Jolt/Renderer/DebugRendererSimple.h>
#endif

#include "calgine/core/physics/physics_world.h"
#include "calgine_pch.h"
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Calgine {

class RigidBody;

// ---- glm <-> Jolt conversions ----

inline JPH::Vec3 to_jolt(const glm::vec3& v) { return JPH::Vec3(v.x, v.y, v.z); }
inline JPH::Quat to_jolt(const glm::quat& q) { return JPH::Quat(q.x, q.y, q.z, q.w).Normalized(); }
inline glm::vec3 to_glm(JPH::Vec3Arg v) { return glm::vec3(v.GetX(), v.GetY(), v.GetZ()); }
inline glm::quat to_glm(JPH::QuatArg q) { return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ()); }
#ifdef JPH_DOUBLE_PRECISION
inline JPH::RVec3 to_jolt_r(const glm::vec3& v) { return JPH::RVec3(v.x, v.y, v.z); }
inline glm::vec3 to_glm(JPH::RVec3Arg v) { return glm::vec3(float(v.GetX()), float(v.GetY()), float(v.GetZ())); }
#else
inline JPH::RVec3 to_jolt_r(const glm::vec3& v) { return to_jolt(v); }
#endif

// ---- Collision layers ----
// Two layers: static bodies never test against each other; everything else collides with everything.

namespace PhysicsLayers
{
  static constexpr JPH::ObjectLayer non_moving = 0;
  static constexpr JPH::ObjectLayer moving = 1;
  static constexpr JPH::ObjectLayer count = 2;
}

namespace PhysicsBroadPhaseLayers
{
  static constexpr JPH::BroadPhaseLayer non_moving(0);
  static constexpr JPH::BroadPhaseLayer moving(1);
  static constexpr JPH::uint count = 2;
}

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
{
public:
  bool ShouldCollide(JPH::ObjectLayer object_1, JPH::ObjectLayer object_2) const override
  {
    if (object_1 == PhysicsLayers::non_moving)
      return object_2 == PhysicsLayers::moving;
    return true;
  }
};

class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
  JPH::uint GetNumBroadPhaseLayers() const override { return PhysicsBroadPhaseLayers::count; }

  JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
  {
    return layer == PhysicsLayers::non_moving ? PhysicsBroadPhaseLayers::non_moving : PhysicsBroadPhaseLayers::moving;
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
  {
    return layer == PhysicsBroadPhaseLayers::non_moving ? "non_moving" : "moving";
  }
#endif
};

class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
  bool ShouldCollide(JPH::ObjectLayer layer_1, JPH::BroadPhaseLayer layer_2) const override
  {
    if (layer_1 == PhysicsLayers::non_moving)
      return layer_2 == PhysicsBroadPhaseLayers::moving;
    return true;
  }
};

// ---- Contact queue ----
// Jolt calls these from its worker threads during PhysicsSystem::Update(), so we only record body IDs
// here. PhysicsWorld::step() turns them into CollisionEvents on the main thread afterwards.

struct QueuedContact
{
  bool is_enter = true;
  JPH::BodyID body_1;
  JPH::BodyID body_2;
  glm::vec3 point = glm::vec3(0.0f);
  glm::vec3 normal = glm::vec3(0.0f);
};

class ContactQueue final : public JPH::ContactListener
{
public:
  void OnContactAdded(const JPH::Body& body_1, const JPH::Body& body_2, const JPH::ContactManifold& manifold, JPH::ContactSettings&) override
  {
    QueuedContact contact;
    contact.is_enter = true;
    contact.body_1 = body_1.GetID();
    contact.body_2 = body_2.GetID();
    if (manifold.mRelativeContactPointsOn1.size() > 0)
      contact.point = to_glm(manifold.GetWorldSpaceContactPointOn1(0));
    contact.normal = to_glm(manifold.mWorldSpaceNormal);

    std::lock_guard<std::mutex> lock(mutex);
    contacts.push_back(contact);
  }

  void OnContactRemoved(const JPH::SubShapeIDPair& pair) override
  {
    QueuedContact contact;
    contact.is_enter = false;
    contact.body_1 = pair.GetBody1ID();
    contact.body_2 = pair.GetBody2ID();

    std::lock_guard<std::mutex> lock(mutex);
    contacts.push_back(contact);
  }

  std::vector<QueuedContact> take()
  {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<QueuedContact> out;
    out.swap(contacts);
    return out;
  }

private:
  std::mutex mutex;
  std::vector<QueuedContact> contacts;
};

// ---- Debug renderer ----

#ifdef JPH_DEBUG_RENDERER
class PhysicsDebugRenderer final : public JPH::DebugRendererSimple
{
public:
  void DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg colour) override;
  void DrawTriangle(JPH::RVec3Arg v1, JPH::RVec3Arg v2, JPH::RVec3Arg v3, JPH::ColorArg colour, ECastShadow cast_shadow) override;
  void DrawText3D(JPH::RVec3Arg, const std::string_view&, JPH::ColorArg, float) override {}
};
#endif

// ---- World state ----

struct PhysicsWorld::Impl
{
  // Falls back to malloc when the preallocated block runs out (plain TempAllocatorImpl aborts).
  std::unique_ptr<JPH::TempAllocatorImplWithMallocFallback> temp_allocator;
  std::unique_ptr<JPH::JobSystemThreadPool> job_system;

  BroadPhaseLayerInterfaceImpl broad_phase_layer_interface;
  ObjectVsBroadPhaseLayerFilterImpl object_vs_broad_phase_filter;
  ObjectLayerPairFilterImpl object_pair_filter;

  std::unique_ptr<JPH::PhysicsSystem> system;
  ContactQueue contact_queue;

#ifdef JPH_DEBUG_RENDERER
  std::unique_ptr<PhysicsDebugRenderer> debug_renderer;
#endif

  /** Body ID (index + sequence) -> owning RigidBody. Only touched on the main thread. */
  std::unordered_map<uint32_t, RigidBody*> bodies;

  /** Last EPhysicsUpdateError flags that were logged, and when (to avoid logging every step). */
  uint32_t last_logged_error = 0;
  std::chrono::steady_clock::time_point last_error_log_time;

  JPH::BodyInterface& body_interface() { return system->GetBodyInterface(); }

  RigidBody* find(const JPH::BodyID& id) const
  {
    auto it = bodies.find(id.GetIndexAndSequenceNumber());
    return it == bodies.end() ? nullptr : it->second;
  }
};

}
