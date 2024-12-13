#include <Eigen/Core>

#include <iostream>
// #include "geometry/space.hpp"
// #include "geometry/transform.hpp"

int main() {
  std::cout << "Hello, this is Software Renderer.\n\n";
  // PerformanceTest();
  // SDL_Window* window = nullptr;
  // if (SDL_Init(SDL_INIT_VIDEO) < 0)
  //   std::cout << "Error during SDL_Init()!\n";
  // window = SDL_CreateWindow("Software renderer", SDL_WINDOWPOS_CENTERED,
  //                           SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);
  // if (window == nullptr)
  //   std::cout << "Error while creating window!\n";
  // SDL_Renderer* renderer =
  //     SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  // SDL_RenderClear(renderer);
  // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  // auto camera = std::make_shared<CameraTransform>();
  // auto perspective =
  //     std::make_shared<PerspectiveProjection>(1, 10, -1, 1, 1, -1);
  // auto viewport = std::make_shared<ViewportTransform>(800, 800, 0, 0);
  // TransformPipeline pipeline(camera, perspective, viewport);
  // Vertex v1(-1, 2, -3), v2(2, 2, -3), v3(2, 2, -6), v4(-1, 2, -6);
  // Vertex v5(-1, -1, -3), v6(2, -1, -3), v7(2, -1, -6), v8(-1, -1, -6);
  // TriangleSharedPointer tr[12];
  // tr[0] = std::make_shared<Triangle>(v2, v1, v5);
  // tr[1] = std::make_shared<Triangle>(v5, v6, v2);
  // tr[2] = std::make_shared<Triangle>(v3, v2, v6);
  // tr[3] = std::make_shared<Triangle>(v6, v7, v3);
  // tr[4] = std::make_shared<Triangle>(v4, v3, v7);
  // tr[5] = std::make_shared<Triangle>(v7, v8, v4);
  // tr[6] = std::make_shared<Triangle>(v1, v4, v8);
  // tr[7] = std::make_shared<Triangle>(v8, v5, v1);
  // tr[8] = std::make_shared<Triangle>(v3, v4, v1);
  // tr[9] = std::make_shared<Triangle>(v1, v2, v3);
  // tr[10] = std::make_shared<Triangle>(v7, v6, v5);
  // tr[11] = std::make_shared<Triangle>(v5, v8, v7);
  // Space world_space;
  // for (size_t i = 0; i < 12; i++)
  //   world_space.EnqueueAddTriangle(tr[i]);
  // world_space.UpdateSpace();

  // float x_pos_start = 0.0, x_pos_end = 1.0;
  // float fps = 60.0;
  // float duration_sec = 20.0;
  // float x_pos_increment = (x_pos_end - x_pos_start) / (fps * duration_sec);

  // for (float x_pos = x_pos_start; x_pos < x_pos_end; x_pos += x_pos_increment) {
  //   SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  //   SDL_RenderClear(renderer);
  //   SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  //   camera->GetCamera().SetLocation({x_pos, 0, 0});
  //   camera->UpdateTransform();
  //   pipeline.RunPipeline(world_space);
  //   Space output_space = pipeline.GetOutputSpace();
  //   for (size_t t = 0; t < output_space.GetTriangleCount(); t++) {
  //     for (size_t a : {0, 1, 2}) {
  //       size_t b = (a + 1) % 3;
  //       int a_x = output_space.GetVertices()(0, t * kVerticesPerTriangle + a);
  //       int a_y = output_space.GetVertices()(1, t * kVerticesPerTriangle + a);
  //       int b_x = output_space.GetVertices()(0, t * kVerticesPerTriangle + b);
  //       int b_y = output_space.GetVertices()(1, t * kVerticesPerTriangle + b);
  //       SDL_RenderDrawLine(renderer, a_x, a_y, b_x, b_y);
  //     }
  //   }
  //   SDL_RenderPresent(renderer);
  // }

  // SDL_DestroyRenderer(renderer);
  // // SDL_DestroyWindowSurface(window);
  // SDL_DestroyWindow(window);
  // SDL_Quit();
  return 0;
}