#include "player.hpp"

Player::Player()
    : Camera({0, 0, 2.9}), speed_(0), movement_direction_(Vector3::Zero()) {}
