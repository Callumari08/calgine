#include "input_system.h"
#include "calgine/core/event_context.h"
#include "calgine/core/input/raw_input.h"

namespace Calgine {

InputSystemBehaviour* InputSystemBehaviour::instance = nullptr;
std::unordered_map<std::string, ActionMap> InputSystemBehaviour::action_maps;
std::string InputSystemBehaviour::active_action_map;

InputSystemBehaviour::InputSystemBehaviour()
{
  if (instance)
  {
    Log::get_engine_logger()->error("There is already an Input System! (You're not supposed to create your own one.)");
    return;
  }
  instance = this;
}

InputSystemBehaviour* InputSystemBehaviour::get_instance()
{
  if (!instance)
  {
    Log::get_engine_logger()->error("There is no active Input System in the application!");
    return nullptr;
  }

  return instance;
}

void InputSystemBehaviour::submit_action_map(const std::string name, const ActionMap action_map, const bool set_active_action_map)
{
  if (name == "")
  {
    Log::get_engine_logger()->error("Cannot set an empty string as an action map!");
  }
  
  action_maps[name] = action_map;

  if (set_active_action_map)
    active_action_map = name;
}

void InputSystemBehaviour::set_active_action_map(const std::string action_map_name)
{
  active_action_map = action_map_name;
}

void InputSystemBehaviour::on_destroy()
{
  if (instance == this)
  {
    Log::get_engine_logger()->error("Input System was Destroyed");
    instance = nullptr;
  }
}

/// Return the "empty" value for a given control type
InputValue InputSystemBehaviour::get_empty_value(ControlType type)
{
  switch (type)
  {
    case ControlType::button:
      return InputButton{false};
    case ControlType::vector1:
      return 0.0f;
    case ControlType::vector2:
      return glm::vec2(0.0f);
    case ControlType::vector3:
      return glm::vec3(0.0f);
    default:
      return InputButton{false};
  }
}

/// Aggregate a contribution into an action value (for combining multiple inputs)
void InputSystemBehaviour::aggregate_contribution(InputValue& target, const InputValue& contribution)
{
  // Only aggregate numeric types that can be combined
  if (std::holds_alternative<float>(target) && std::holds_alternative<float>(contribution))
  {
    std::get<float>(target) += std::get<float>(contribution);
  }
  else if (std::holds_alternative<glm::vec2>(target) && std::holds_alternative<glm::vec2>(contribution))
  {
    std::get<glm::vec2>(target) += std::get<glm::vec2>(contribution);
  }
  else if (std::holds_alternative<glm::vec3>(target) && std::holds_alternative<glm::vec3>(contribution))
  {
    std::get<glm::vec3>(target) += std::get<glm::vec3>(contribution);
  }
  // InputButton can't be aggregated, just OR the boolean values
  else if (std::holds_alternative<InputButton>(target) && std::holds_alternative<InputButton>(contribution))
  {
    auto& target_btn = std::get<InputButton>(target);
    const auto& contrib_btn = std::get<InputButton>(contribution);
    target_btn.is_pressed = target_btn.is_pressed || contrib_btn.is_pressed;
  }
}

// Handles discrete event-based input (e.g., mouse button clicks)
void InputSystemBehaviour::update_action(const InputAction& action, const RawInputEvent* input_event, EventContext& event_context)
{
  // Try to find a matching mapping for this raw input
  for (const InputMapping& mapping : action.mappings)
  {
    // Type must match
    if (mapping.type != input_event->type)
      continue;
    
    bool matches_press = false;
    bool matches_release = false;
    
    // Check if this specific input matches the mapping
    switch (input_event->type)
    {
      case RawInputEventType::keyboard:
      {
        const auto& kbd = std::get<RawKeyboardData>(input_event->data);
        if (std::holds_alternative<SDL_Scancode>(mapping.matcher))
        {
          SDL_Scancode expected = std::get<SDL_Scancode>(mapping.matcher);
          matches_press = (kbd.scancode == expected && kbd.is_pressed && !kbd.is_repeat);
          matches_release = (kbd.scancode == expected && !kbd.is_pressed);
        }
        break;
      }
      
      case RawInputEventType::mouse_button:
      {
        const auto& mouse = std::get<RawMouseButtonData>(input_event->data);
        if (std::holds_alternative<Uint8>(mapping.matcher))
        {
          Uint8 expected = std::get<Uint8>(mapping.matcher);
          matches_press = (mouse.button == expected && mouse.is_pressed);
          matches_release = (mouse.button == expected && !mouse.is_pressed);
        }
        break;
      }
      
      case RawInputEventType::mouse_move:
      {
        // Always match for mouse move (no specific matcher needed)
        matches_press = true;
        break;
      }
      
      case RawInputEventType::mouse_wheel:
      {
        // Always match for mouse wheel (no specific matcher needed)
        matches_press = true;
        break;
      }
    }
    
    if (!matches_press && !matches_release)
      continue;
    
    // This mapping matched! Submit the contribution
    InputActionEvent action_event;
    action_event.submit_tick = TickType::update;
    action_event.type = action.control_type;
    
    // For mouse move/wheel, use the actual event data; for releases, use empty value
    if (input_event->type == RawInputEventType::mouse_move)
    {
      const auto& mouse_move = std::get<RawMouseMoveData>(input_event->data);
      action_event.value = glm::vec2(mouse_move.relative_motion.x, mouse_move.relative_motion.y);
    }
    else if (input_event->type == RawInputEventType::mouse_wheel)
    {
      const auto& mouse_wheel = std::get<RawMouseWheelData>(input_event->data);
      action_event.value = glm::vec2(mouse_wheel.x_direction, mouse_wheel.y_direction);
    }
    else if (matches_release)
    {
      action_event.value = this->get_empty_value(action.control_type);
    }
    else
    {
      action_event.value = mapping.contribution;
    }
    
    event_context.submit(action_event);
  }
}

void InputSystemBehaviour::update_tick(EventContext& event_context)
{
  if (active_action_map == "") 
    return;

  // Process discrete input events (mouse clicks, etc)
  for (const RawInputEvent* input_event : event_context.get_events<RawInputEvent>())
  {
    for (auto const& [name, input_action] : action_maps[active_action_map].map)
    {
      update_action(input_action, input_event, event_context);
    }
  }

  // Poll keyboard state every frame for continuous input (movement)
  int num_keys = 0;
  const Uint8* keyboard_state = reinterpret_cast<const Uint8*>(SDL_GetKeyboardState(&num_keys));

  for (auto const& [action_name, input_action] : action_maps[active_action_map].map)
  {
    // Only aggregate keyboard mappings (not mouse/etc)
    InputActionEvent action_event;
    action_event.submit_tick = TickType::update;
    action_event.type = input_action.control_type;
    action_event.value = this->get_empty_value(input_action.control_type);

    bool has_input = false;

    for (const InputMapping& mapping : input_action.mappings)
    {
      // Only handle keyboard mappings here
      if (mapping.type != RawInputEventType::keyboard)
        continue;

      if (std::holds_alternative<SDL_Scancode>(mapping.matcher))
      {
        SDL_Scancode scancode = std::get<SDL_Scancode>(mapping.matcher);
        if (keyboard_state[scancode])
        {
          has_input = true;
          this->aggregate_contribution(action_event.value, mapping.contribution);
        }
      }
    }

    // Only emit if at least one key is pressed
    if (has_input)
    {
      event_context.submit(action_event);
    }
  }
}

}