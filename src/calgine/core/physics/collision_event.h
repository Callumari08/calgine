#pragma once

#include <glm/glm.hpp>
#include "calgine/core/event_data.h"
#include "calgine_api.h"

namespace Calgine {

class GameObject;
class RigidBody;

/**
 * @brief Submitted to the EventContext by the physics system when two RigidBodies start or stop touching.
 *
 * @details
 * Events are submitted during the fixed_update phase (right after the physics step) and stay readable
 * for the rest of that frame: later fixed steps, update, late_update, render, imgui_render and final.
 * They are cleared when the next frame enters fixed_update.
 *
 * @code{.cpp}
 * void update_tick(EventContext& event_context) override
 * {
 *   for (const CollisionEvent* collision : event_context.get_events<CollisionEvent>())
 *   {
 *     if (collision->type == CollisionEvent::enter && collision->involves(get_game_object()))
 *       print("Hit {}", collision->other(get_game_object())->get_name());
 *   }
 * }
 * @endcode
 *
 * @note For exit events, contact_point and normal are zero (Jolt doesn't report them).
 */
struct CALGINE_API CollisionEvent : public EventData
{
  enum Type
  {
    enter,
    exit,
  };

  Type type = enter;

  GameObject* a = nullptr;
  GameObject* b = nullptr;
  RigidBody* body_a = nullptr;
  RigidBody* body_b = nullptr;

  /** @brief World-space contact point (enter only). */
  glm::vec3 contact_point = glm::vec3(0.0f);
  /** @brief World-space contact normal, pointing from a towards b (enter only). */
  glm::vec3 normal = glm::vec3(0.0f);

  bool involves(const GameObject* game_object) const
  {
    return game_object && (a == game_object || b == game_object);
  }

  /** @brief The other GameObject in the collision, or nullptr if @p game_object isn't involved. */
  GameObject* other(const GameObject* game_object) const
  {
    if (a == game_object) return b;
    if (b == game_object) return a;
    return nullptr;
  }
};

}
