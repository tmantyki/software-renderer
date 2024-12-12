#include <SDL2/SDL.h>
#include <Eigen/Core>

#include <chrono>
#include <iostream>
#include "geometry/space.hpp"
#include "geometry/transform.hpp"
#include "utility/timer.hpp"

void PerformanceTest() {
  /* 4 by 1K matrix multiplication */
  constexpr size_t length = 1000000;
  Eigen::Matrix<float, 4, 4> A = Eigen::Matrix<float, 4, 4>::Random();
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> B, C;
  B = Eigen::Matrix<float, length, 4>::Random();
  Timer timer_multiplication("4 by 1M matrix multiplication");
  timer_multiplication.Start();
  C = B * A;
  timer_multiplication.Stop(true);

  /* Transformation of 1M vectors in four dimensions */
  constexpr size_t vector_count = 1000000;
  Eigen::Matrix<float, 4, 4> M = Eigen::Matrix<float, 4, 4>::Random();
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> V;
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> S;
  V.resize(4, vector_count);
  S.resize(4, vector_count);
  V = Eigen::Matrix<float, 4, vector_count>::Random();
  Timer timer_product("Transform 1M vectors");
  timer_product.Start();
  S = M * V;
  timer_product.Stop(true);

  /* Resize vector matrix by 1M-1 columns */
  Timer timer_resize_minus("Resize 1M-1 columns");
  timer_resize_minus.Start();
  V.resize(4, vector_count - 1);
  timer_resize_minus.Stop(true);

  /* Resize vector matrix by 1M+1 columns */
  Timer timer_resize_plus("Resize 1M+1 columns");
  timer_resize_plus.Start();
  V.resize(4, vector_count);
  timer_resize_plus.Stop(true);

  /* Conservative resize vector matrix by 1M-1 columns */
  Timer timer_conservative_resize_minus("Conservative Resize 1M-1 columns");
  timer_conservative_resize_minus.Start();
  V.resize(4, vector_count - 1);
  timer_conservative_resize_minus.Stop(true);

  /* Conservative resize vector matrix by 1M+1 columns */
  Timer timer_conservative_resize_plus("Conservative Resize 1M+1 columns");
  timer_conservative_resize_plus.Start();
  V.resize(4, vector_count);
  timer_conservative_resize_plus.Stop(true);
}

int main() {
  std::cout << "Hello, this is Software Renderer.\n\n";
  PerformanceTest();
  SDL_Window* window = nullptr;
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    std::cout << "Error during SDL_Init()!\n";
  window = SDL_CreateWindow("Software renderer", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);
  if (window == nullptr)
    std::cout << "Error while creating window!\n";
  // SDL_Surface* surface = SDL_GetWindowSurface(window);
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  auto camera = std::make_shared<CameraTransform>();
  auto perspective =
      std::make_shared<PerspectiveProjection>(1, 10, -1, 1, 1, -1);
  auto viewport = std::make_shared<ViewportTransform>(800, 800, 0, 0);
  TransformPipeline pipeline(camera, perspective, viewport);
  Vertex v1(-1, 1, -3), v2(2, 1, -3), v3(0, -1, -3);
  auto tr = std::make_shared<Triangle>(v1, v2, v3);
  Space world_space;
  world_space.EnqueueAddTriangle(tr);
  world_space.UpdateSpace();
  pipeline.RunPipeline(world_space);
  Space output_space = pipeline.GetOutputSpace();
  for (size_t t = 0; t < output_space.GetTriangleCount(); t++) {
    for (size_t a : {0, 1, 2}) {
      size_t b = (a + 1) % 3;
      int a_x = output_space.GetVertices()(0, t * kVerticesPerTriangle + a);
      int a_y = output_space.GetVertices()(1, t * kVerticesPerTriangle + a);
      int b_x = output_space.GetVertices()(0, t * kVerticesPerTriangle + b);
      int b_y = output_space.GetVertices()(1, t * kVerticesPerTriangle + b);
      SDL_RenderDrawLine(renderer, a_x, a_y, b_x, b_y);
    }
  }
  SDL_RenderPresent(renderer);
  
  SDL_Delay(5000);
  SDL_DestroyRenderer(renderer);
  // SDL_DestroyWindowSurface(window);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}