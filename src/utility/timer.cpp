
#include "timer.hpp"
#include <cassert>
#include <iostream>

using namespace std::chrono;

Timer::Timer(const char* label) : label_(label) {}

void Timer::Start() noexcept {
  assert(!active_);
  duration_ns_ = 0;
  active_ = true;
  reference_ = high_resolution_clock::now();
}

void Timer::Pause() noexcept {
  auto end = high_resolution_clock::now();
  duration_ns_ += duration_cast<nanoseconds>(end - reference_).count();
  assert(active_);
  active_ = false;
}

void Timer::Continue() noexcept {
  assert(!active_);
  active_ = true;
  reference_ = high_resolution_clock::now();
}

void Timer::Stop(bool print_results) noexcept {
  if (active_) {
    auto end = high_resolution_clock::now();
    duration_ns_ += duration_cast<nanoseconds>(end - reference_).count();
    active_ = false;
  }
  if (print_results)
    Print();
}

void Timer::Print() const noexcept {
  assert(!active_);
  std::cout << label_ << ": " << GetDuration() << " ms\n";
}

float Timer::GetDuration() const noexcept {
  assert(!active_);
  return duration_ns_ / 1000000.0;
}