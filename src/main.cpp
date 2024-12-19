#include <iostream>

#include "server/game_state.hpp"
#include "ui/controller.hpp"
#include "ui/ui.hpp"

int main() {
  std::cout << "Hello, this is Software Renderer.\n\n";
  UserInterface user_interface;

  GameState game_state;
  user_interface.InitializeSdlObjects();
  Controller controller;
  Camera camera;
  do {
    controller.UpdateState();
    controller.OffsetCamera(camera);
    SDL_Delay(10);
  } while (!controller.CheckQuitRequest());
  user_interface.DestroySdlObjects();
  return 0;
}