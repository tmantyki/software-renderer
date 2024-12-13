#ifndef UI_HPP
#define UI_HPP

#include <cstdint>

constexpr int16_t kCentered = -1;

class UserInterface {
 public:
  UserInterface();
  UserInterface(int16_t x_position,
                int16_t y_position,
                uint16_t width,
                uint16_t height);
  bool CreateWindow();
  void DestroyWindow();
 private:
  const uint16_t x_position_;
  const uint16_t y_position_;
  const uint16_t width_;
  const uint16_t height_;
  bool window_created_;
};

#endif