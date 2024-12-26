#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "geometry/space.hpp"
#include "geometry/transform.hpp"
#include "server/player.hpp"
#include "ui/controller.hpp"

class GameState {
 public:
  GameState();
  void ProcessTick() noexcept;
  void UpdatePlayerState(const Controller& controller) noexcept;
  const Space& GetOutputSpace() const noexcept;

 private:
  Player player_;
  CameraTransform camera_transform_;
  PerspectiveProjection perspective_;
  ViewportTransform viewport_;
  TransformPipeline pipeline_;
  Space world_space_;
  uint64_t tick_counter_;
};

#endif