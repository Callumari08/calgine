#include "camera.h"

namespace Calgine {

Camera::Camera(float fov, float aspect_ratio, float near_plane, float far_plane)
  : fov(fov), aspect_ratio(aspect_ratio), near_plane(near_plane), far_plane(far_plane)
{
  recalculate_projection();
}

void Camera::update(const Transform& transform) {
  glm::vec3 position = transform.position;
  
  glm::mat4 rot_matrix = glm::mat4(1.0f);
  rot_matrix = glm::rotate(rot_matrix, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  rot_matrix = glm::rotate(rot_matrix, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  rot_matrix = glm::rotate(rot_matrix, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  
  glm::vec3 forward = -glm::vec3(rot_matrix[2]);
  glm::vec3 up = glm::vec3(rot_matrix[1]);
  
  view_matrix = glm::lookAt(position, position + forward, up);
}

void Camera::set_fov(float new_fov)
{
  fov = new_fov;
  recalculate_projection();
}

void Camera::set_aspect_ratio(float new_aspect)
{
  aspect_ratio = new_aspect;
  recalculate_projection();
}

void Camera::set_near_plane(float new_near)
{
  near_plane = new_near;
  recalculate_projection();
}

void Camera::set_far_plane(float new_far)
{
  far_plane = new_far;
  recalculate_projection();
}

void Camera::recalculate_projection()
{
  projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
}

} // namespace Calgine
