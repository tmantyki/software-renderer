#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <string>

class Timer {
 public:
  Timer() = delete;
  Timer(const char* label);
  void Start();
  void Pause();
  void Continue();
  void Stop(bool print_results);
  void Print() const;
  float GetDuration() const;

 private:
  const std::string label_;
  int64_t duration_us_ = 0;
  std::chrono::high_resolution_clock::time_point reference_;
  bool active_ = false;
};

#endif