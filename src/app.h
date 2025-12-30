#ifndef APP_H
#define APP_H

#include <string>

/**
 * @brief This is the class from which all user applications derive from.
 *
 * @details
 * To get started with programming an application, create your own class that
 * derives from `App`.
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
public:
  std::string app_name = "Calgine Game";

  void systems_init();
  void main_loop();
};

#endif