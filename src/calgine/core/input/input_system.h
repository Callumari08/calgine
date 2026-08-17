#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/event_context.h"
#include "calgine/core/event_data.h"
#include "calgine/core/input/raw_input.h"
#include "calgine_pch.h"
#include "calgine_api.h"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <SDL3/SDL.h>

namespace Calgine {

enum CALGINE_API ControlType 
{
  button,
  vector1, // floating point number
  vector2,
  vector3,
};

union CALGINE_API InputButton
{
  bool is_pressed;
};

using InputValue = std::variant<InputButton, float, glm::vec2, glm::vec3>;

struct CALGINE_API InputActionEvent : EventData
{
  ControlType type;
  InputValue value;
};

struct CALGINE_API InputMapping
{
  RawInputEventType type;
  std::variant<SDL_Scancode, Uint8> matcher;
  InputValue contribution;
};

struct CALGINE_API InputAction
{
  std::string name;
  ControlType control_type;
  std::vector<InputMapping> mappings;
};

struct CALGINE_API ActionMap
{
  std::string name;
  // name of InputAction to InputAction
  std::unordered_map<std::string, InputAction> map;
};

class CALGINE_API InputSystemBehaviour : public Behaviour
{
public:
  InputSystemBehaviour();

  static InputSystemBehaviour* get_instance();
  static void set_action_maps(std::unordered_map<std::string, ActionMap>& map, std::string active_map = "")
  {
    action_maps = map;
    
    if (active_map != "")
      active_action_map = active_map;
  };

  static void submit_action_map(const std::string name, const ActionMap action_map, const bool set_active_action_map = false);
  static void set_active_action_map(const std::string action_map_name);

private:

  void update_action(const InputAction& action, const RawInputEvent* input_event, EventContext& event_context);
  
  void update_tick(EventContext& event_context) override;

  InputValue get_empty_value(ControlType type);
  void aggregate_contribution(InputValue& target, const InputValue& contribution);

  void on_destroy() override;

  static InputSystemBehaviour* instance;

  // name of ActionMap to ActionMap
  static std::unordered_map<std::string, ActionMap> action_maps;
  static std::string active_action_map;
};

CALGINE_REGISTER_BEHAVIOUR(InputSystemBehaviour, "input_system");

}