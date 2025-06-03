#include "plane.hpp"

void Plane::NormalizeVector() {
  vector_normalized_ = vector_;
  vector_normalized_[3] = 0;
  float norm = vector_normalized_.norm();
  vector_normalized_ /= norm;
  vector_normalized_[3] = vector_[3] / norm;
}
