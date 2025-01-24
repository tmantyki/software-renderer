#include <iostream>

#include "server/game_state.hpp"
#include "ui/controller.hpp"
#include "ui/rasterizer.hpp"
#include "ui/ui.hpp"
#include "utility/timer.hpp"

int main(int argc, char** argv) {
  std::cout << "Hello, this is Software Renderer.\n\n";

  int max_ticks = -1;
  if (argc > 2)
    if (!std::strcmp(argv[1], "--maxticks"))
      std::sscanf(argv[2], "%d", &max_ticks);

  // UserInterface user_interface;
  BenchmarkInterface user_interface;
  Controller controller;
  GameState game_state;
  WireframeRasterizer wirefreame_rasterizer;
  // FlatRasterizer rasterizer;
  TexturedRasterizer rasterizer;
  Rasterizer* active_rasterizer = &rasterizer;
  Timer timer("main()");
  timer.Start();

  while (true) {
    controller.UpdateState();
    if (controller.CheckQuitRequest()) {
      std::cout << "Ticks: " << game_state.GetTick() << "\n";
      break;
    }
    if (controller.ConsumeToggleRasterizerRequest()) {
      SDL_Delay(100);
      if (active_rasterizer == &rasterizer)
        active_rasterizer = &wirefreame_rasterizer;
      else
        active_rasterizer = &rasterizer;
    }
    game_state.UpdatePlayerState(controller);
    game_state.ProcessTick();
    active_rasterizer->RasterizeGameState(game_state, user_interface);
    user_interface.RenderPresent();
    if (max_ticks > 0 &&
        game_state.GetTick() > static_cast<uint64_t>(max_ticks))
      break;
  }

  timer.Stop(false);
  std::cout << "Mean FPS: " << game_state.GetTick() * 1000 / timer.GetDuration()
            << "\n";
  return 0;
}