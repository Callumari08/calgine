#pragma once

#include "calgine_pch.h"
#include "calgine_api.h"

namespace Calgine {

/**
 * @brief This is the class from which all user applications derive from.
 *
 * @details
 * To get started with programming an application, create your own class that
 * derives from `App`.
 *
 * @warning This example is out of date. Please do not use it to understand how behaviours work.
 *
 * @code{.cpp}
 * class MyApp : public Calgine::App
 * {
 * private:
 *   std::string app_name = "My App";
 * public:
 * std::string get_app_name() override
 * {
 *   return app_name;
 * }
 *   // stuff related to your app can go here
 * }
 *
 * class MyBehaviour : public Calgine::Behaviour
 * {
 *  void start() override
 *  {
 *    std::cout << "MyBehaviour Start!";
 *  }
 *
 *  void tick() override
 *  {
 *    i++;
 *    std::cout << i << "\n";
 *  }
 * };
 *
 * int main()
 * {
 *   MyApp myApp;
 *
 *  // Intializes libraries
 *  myApp.systems_init();
 *
 *  // add behaviours here
 *  MyBehaviour myBehaviour;
 *
 *  myApp.main_loop();
 *
 *  return 0;
 * }
 * @endcode
 */
class CALGINE_API App // Abstract
{
public:
  virtual std::string get_app_name() = 0;

  void systems_init();
  void main_loop();

private:
  void handle_sdl_events(bool& running);
  void render_windows();
};

} // namespace Calgine