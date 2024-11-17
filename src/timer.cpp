
#include "timer.hpp"
#include <cassert>
#include <iostream>

using namespace std::chrono;

Timer::Timer(const char* label) : label_(label) {}

void Timer::Start() {
  assert(!active_);
  duration_us_ = 0;
  active_ = true;
  reference_ = high_resolution_clock::now();
}

void Timer::Pause() {
  auto end = high_resolution_clock::now();
  duration_us_ += duration_cast<microseconds>(end - reference_).count();
  assert(active_);
  active_ = false;
}

void Timer::Continue() {
  assert(!active_);
  active_ = true;
  reference_ = high_resolution_clock::now();
}

void Timer::Stop(bool print_results) {
  auto end = high_resolution_clock::now();
  duration_us_ += duration_cast<microseconds>(end - reference_).count();
  assert(active_);
  active_ = false;
  if (print_results)
    Print();
}

void Timer::Print() const {
  assert(!active_);
  std::cout << label_ << ": " << GetDuration() << " ms\n";
}

float Timer::GetDuration() const {
  assert(!active_);
  return duration_us_ / 1000.0;
}