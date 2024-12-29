#include "direction.hpp"

Direction::Direction(float x, float y, float z) : Coordinate(x, y, z, 0) {}

Direction::Direction(Vector3 vector)
    : Coordinate(vector[0], vector[1], vector[2], 0) {}

Direction::Direction(Vector4 vector)
    : Coordinate(vector) {}