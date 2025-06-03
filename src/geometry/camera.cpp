#include "camera.hpp"

bool Camera::operator==(const Camera& rhs) const {
  if (this->GetLocation() != rhs.GetLocation())
    return false;
  if (this->GetPitch() != rhs.GetPitch())
    return false;
  if (this->GetYaw() != rhs.GetYaw())
    return false;
  if (this->GetRoll() != rhs.GetRoll())
    return false;
  return true;
}