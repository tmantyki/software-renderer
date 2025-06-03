#pragma once

#include <cassert>
#include <chrono>
#include <string>

using namespace std::chrono;

class Timer {
 public:
  Timer() = delete;
  Timer(const char* label) : label_(label) {}
  void Start() noexcept {
    assert(!active_);
    duration_ns_ = 0;
    active_ = true;
    reference_ = high_resolution_clock::now();
  }
  void Pause() noexcept {
    auto end = high_resolution_clock::now();
    duration_ns_ += duration_cast<nanoseconds>(end - reference_).count();
    assert(active_);
    active_ = false;
  }
  void Continue() noexcept {
    assert(!active_);
    active_ = true;
    reference_ = high_resolution_clock::now();
  }
  void Stop(bool print_results) noexcept {
    if (active_) {
      auto end = high_resolution_clock::now();
      duration_ns_ += duration_cast<nanoseconds>(end - reference_).count();
      active_ = false;
    }
    if (print_results)
      Print();
  }
  void Print() const noexcept {
    assert(!active_);
    std::cout << label_ << ": " << GetDuration() << " ms\n";
  }
  float GetDuration() const noexcept {
    assert(!active_);
    return duration_ns_ / 1000000.0;
  }

 private:
  const std::string label_;
  int64_t duration_ns_ = 0;
  std::chrono::high_resolution_clock::time_point reference_;
  bool active_ = false;
};
