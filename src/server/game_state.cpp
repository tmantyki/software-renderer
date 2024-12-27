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
  float x = static_cast<float>(controller.GetX());
  float y = static_cast<float>(controller.GetY());
  float z = static_cast<float>(controller.GetZ());
  float pitch = static_cast<float>(controller.GetPitch());
  float yaw = static_cast<float>(controller.GetYaw());
  float roll = static_cast<float>(controller.GetRoll());
  Direction translation_direction(x, y, z);
  Vector4 translation_vector =
      camera_transform_.GetMatrixInverse() * translation_direction.GetVector();
  translation_vector *= kTranslationIncrement;
  camera.SetLocation(Vector3(camera.GetLocation().GetVector()({0, 1, 2}) +
                             translation_vector({0, 1, 2})));
  camera.SetPitch(camera.GetPitch() + pitch * kAngularIncrement);
  camera.SetYaw(camera.GetYaw() + yaw * kAngularIncrement);
  camera.SetRoll(camera.GetRoll() + roll * kAngularIncrement);
  camera_transform_.UpdateTransform();
}

const Space& GameState::GetOutputSpace() const noexcept {
  return pipeline_.GetOutputSpace();
}