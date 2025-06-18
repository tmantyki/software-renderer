#include <iostream>

#include "rasterizer/rasterizer.hpp"
#include "rasterizer/render_buffer.hpp"
#include "server/game_state.hpp"
#include "ui/controller.hpp"
#include "ui/ui.hpp"
#include "utility/timer.hpp"

int main(int argc, char** argv) {
  std::cout << "Hello, this is Software Renderer.\n\n";

  int max_ticks = -1;
  if (argc > 2)
    if (!std::strcmp(argv[1], "--maxticks"))
      std::sscanf(argv[2], "%d", &max_ticks);

  UserInterface user_interface;
  // BenchmarkInterface user_interface;
  Controller controller;
  GameState game_state;
  Texture default_texture("assets/blender/porcelain.png");
  RenderBuffer render_buffer(user_interface.GetPitch());
  // WireframeRasterizer wireframe_rasterizer;
  // FlatRasterizer flat_rasterizer;
  TexturedRasterizer rasterizer;
  // PureRasterizer rasterizer;
  // Rasterizer* active_rasterizer = &rasterizer;
  RasterizationContext context = {game_state, render_buffer, default_texture};
  Timer timer("main()");
  timer.Start();

  while (true) {
    controller.UpdateState();
    if (controller.CheckQuitRequest()) {
      std::cout << "Ticks: " << game_state.GetTick() << "\n";
      break;
    }
    if (controller.ConsumeRasterizerToggleRequest()) {
      // SDL_Delay(100);
      // if (active_rasterizer == &rasterizer)
      //   active_rasterizer = &wireframe_rasterizer;
      // else
      //   active_rasterizer = &rasterizer;
    }
    game_state.UpdatePlayerState(controller);
    game_state.ProcessTick();

    // wireframe_rasterizer.RasterizeGameState(game_state, render_buffer);
    rasterizer.RasterizeGameState(context);

    // active_rasterizer->RasterizeGameState(game_state, user_interface);
    user_interface.RenderPresent(render_buffer);
    if (max_ticks > 0 &&
        game_state.GetTick() > static_cast<uint64_t>(max_ticks))
      break;
  }

  timer.Stop(false);
  std::cout << "Mean FPS: " << game_state.GetTick() * 1000 / timer.GetDuration()
            << "\n";
  return 0;
}