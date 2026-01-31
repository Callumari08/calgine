#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "calgine/core/transform.h"

namespace Calgine {

class Camera
{
public:
  Camera(
    float fov = 65.0f,
    float aspect_ratio = 16.0f / 9.0f,
    float near_plane = 0.1f,
    float far_plane = 1000.0f
  );

  void update(const Transform& transform);
  
  void set_fov(float fov, bool recalculate = true);
  void set_aspect_ratio(float aspect, bool recalculate = true);
  void set_near_plane(float near, bool recalculate = true);
  void set_far_plane(float far, bool recalculate = true);

  inline const glm::mat4& get_view_matrix() const { return view_matrix; }
  inline const glm::mat4& get_projection_matrix() const { return projection_matrix; }

private:
  glm::mat4 view_matrix;
  glm::mat4 projection_matrix;

  float fov;
  float aspect_ratio;
  float near_plane;
  float far_plane;

  void recalculate_projection();
};

} // namespace Calgine
