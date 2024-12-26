#include "game_state.hpp"
#include "utility/geometry_importer.hpp"

GameState::GameState()
    : camera_transform_(player_),
      perspective_(1, 10, -1, 1, 1, -1),
      viewport_(800, 800),
      pipeline_(camera_transform_, perspective_, viewport_),
      tick_counter_(0) {
  ObjGeometryImporter obj_importer(world_space_);
  obj_importer.ImportGeometryFromFile("assets/teapot.obj");
}

void GameState::ProcessTick() noexcept {
  pipeline_.RunPipeline(world_space_);
  tick_counter_++;
}

void GameState::UpdatePlayerState(const Controller& controller) noexcept {
  Camera& camera = camera_transform_.GetCamera();
  const float translation_offset = 0.01;
  float x = static_cast<float>(controller.GetX());
  float y = static_cast<float>(controller.GetY());
  float z = static_cast<float>(controller.GetZ());
  Vector3 translation_vector = {x, y, z};
  translation_vector *= translation_offset;
  camera.SetLocation(Vector3(camera.GetLocation().GetVector()({0, 1, 2}) +
                             translation_vector));
  camera_transform_.UpdateTransform();
}

const Space& GameState::GetOutputSpace() const noexcept {
  return pipeline_.GetOutputSpace();
}