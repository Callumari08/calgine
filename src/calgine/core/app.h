#pragma once

#include <string>

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
 * class MyApp : public App
 * {
 *   std::string app_name = "My App";
 *   // stuff related to your app can go here
 * }
 *
 * class MyBehaviour : public Behaviour
 * {
 *  int i = 0;
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
class App
{
private:
  std::string app_name = "A Calgine App";
public:
  virtual std::string get_app_name()
  {
    return app_name;
  }

  void systems_init();
  void main_loop();
};