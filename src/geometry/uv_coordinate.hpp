#ifndef UV_COORDINATE_HPP
#define UV_COORDINATE_HPP

class UVCoordinate {
 public:
  UVCoordinate() noexcept;
  UVCoordinate(const float u, const float v) noexcept;
  float GetCoordinateU() const noexcept;
  float GetCoordinateV() const noexcept;

 private:
  float u_;
  float v_;
};

#endif