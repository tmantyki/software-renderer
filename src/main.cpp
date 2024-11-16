#include <Eigen/Core>
#include <chrono>
#include <iostream>
#include "timer.hpp"

void PerformanceTest() {
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
  std::cout << "\nExiting..\n";
  return 0;
}
