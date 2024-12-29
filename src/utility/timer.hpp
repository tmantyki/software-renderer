#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <string>

class Timer {
 public:
  Timer() = delete;
  Timer(const char* label);
  void Start() noexcept;
  void Pause() noexcept;
  void Continue() noexcept;
  void Stop(bool print_results) noexcept;
  void Print() const noexcept;
  float GetDuration() const noexcept;

 private:
  const std::string label_;
  int64_t duration_ns_ = 0;
  std::chrono::high_resolution_clock::time_point reference_;
  bool active_ = false;
};

#endif