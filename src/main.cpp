#include <iostream>

#include "ui/controller.hpp"
#include "ui/ui.hpp"

int main() {
  std::cout << "Hello, this is Software Renderer.\n\n";
  UserInterface user_interface;
  user_interface.InitializeSdlObjects();
  Controller controller;
  Camera camera;
  for (;;) {
    controller.UpdateState();
    if (controller.CheckQuitRequest())
      break;
    controller.OffsetCamera(camera);
    SDL_Delay(10);
  }
  user_interface.DestroySdlObjects();
  return 0;
}