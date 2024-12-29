#include <iostream>

#include "server/game_state.hpp"
#include "ui/controller.hpp"
#include "ui/rasterizer.hpp"
#include "ui/ui.hpp"

int main() {
  std::cout << "Hello, this is Software Renderer.\n\n";

  UserInterface user_interface;
  user_interface.InitializeSdlObjects();
  Controller controller;
  GameState game_state;
  WireframeRasterizer wirefreame_rasterizer;
  FlatRasterizer flat_rasterizer({0.13, -1, 0.49});
  Rasterizer* active_rasterizer = &flat_rasterizer;

  while (true) {
    controller.UpdateState();
    if (controller.CheckQuitRequest()) {
      std::cout << "Ticks: " << game_state.GetTick() << "\n";
      break;
    }
    if (controller.ConsumeToggleRasterizerRequest()) {
      if (active_rasterizer == &flat_rasterizer)
        active_rasterizer = &wirefreame_rasterizer;
      else
        active_rasterizer = &flat_rasterizer;
    }
    game_state.UpdatePlayerState(controller);
    game_state.ProcessTick();
    active_rasterizer->RasterizeGameState(game_state, user_interface);
    SDL_RenderPresent(user_interface.GetSdlRenderer());
  }
  return 0;
}