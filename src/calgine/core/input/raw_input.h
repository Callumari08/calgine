#pragma once

#include "calgine_pch.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_scancode.h"
#include "calgine_api.h"
#include "calgine/core/event_data.h"
#include "glm/ext/vector_int2.hpp"

namespace Calgine {

enum CALGINE_API RawInputEventType
{
  keyboard,
  mouse_button,
  mouse_move,
  mouse_wheel,
  //gamepad,
};

struct CALGINE_API RawInputData
{
  RawInputEventType type;
  virtual ~RawInputData() = default;
};

struct CALGINE_API RawKeyboardData : public RawInputData
{
  SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
  SDL_Keycode keycode = SDLK_UNKNOWN;
  bool is_pressed = false;
  bool is_repeat = false;
  
  RawKeyboardData() = default;
  RawKeyboardData(SDL_Scancode scancode, SDL_Keycode keycode, bool is_pressed, bool is_repeat)
    : scancode(scancode), keycode(keycode), is_pressed(is_pressed), is_repeat(is_repeat) {}
};

struct CALGINE_API RawMouseButtonData : public RawInputData
{
  Uint8 button = 0;
  bool is_pressed = false;
  Uint8 clicks = 0;
  glm::ivec2 position = {0, 0};
  
  RawMouseButtonData() = default;
  RawMouseButtonData(Uint8 button, bool is_pressed, Uint8 clicks, glm::ivec2 position)
    : button(button), is_pressed(is_pressed), clicks(clicks), position(position) {}
};

struct CALGINE_API RawMouseMoveData : public RawInputData
{
  glm::ivec2 position = {0, 0};
  glm::ivec2 relative_motion = {0, 0};
  
  RawMouseMoveData() = default;
  RawMouseMoveData(glm::ivec2 position, glm::ivec2 relative_motion)
    : position(position), relative_motion(relative_motion) {}
};

struct CALGINE_API RawMouseWheelData : public RawInputData
{
  float y_direction = 0.0f;
  float x_direction = 0.0f;
  
  RawMouseWheelData() = default;
  RawMouseWheelData(float y_direction, float x_direction)
    : y_direction(y_direction), x_direction(x_direction) {}
};

struct CALGINE_API RawInputEvent : public EventData
{
  RawInputEventType type;
  std::variant<RawKeyboardData, RawMouseButtonData, RawMouseMoveData, RawMouseWheelData> data;
};

}