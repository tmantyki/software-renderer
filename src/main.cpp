#include <Eigen/Core>
#include <chrono>
#include <iostream>

constexpr size_t vector_count = 10000;

void PerformanceTest() {
  Eigen::Matrix<float, 4, 4> M = Eigen::Matrix<float, 4, 4>::Random();
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> V;
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> S;
  V.resize(4, vector_count);
  S.resize(4, vector_count);
  V = Eigen::Matrix<float, 4, vector_count>::Random();
  auto start = std::chrono::high_resolution_clock::now();
  S = M * V;
  auto end = std::chrono::high_resolution_clock::now();
  float duration_ms =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();
  duration_ms /= 1000.0;
  std::cout << "Measured duration: " << duration_ms << " ms\n\n";
}

int main() {
  std::cout << "Hello, this is Software Renderer.\n"
            << "Running performance test with " << Eigen::nbThreads()
            << " threads and " << vector_count << " vertices..\n\n";
  PerformanceTest();
  std::cout << "Exiting..\n";
  return 0;
}
