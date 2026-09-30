#define GLM_ENABLE_EXPERIMENTAL

#include "transform.h"
#include "glm/ext/vector_float3.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Calgine {

Transform::Transform(const glm::vec3 _position, const glm::vec3 _rotation, const glm::vec3 _scale)
  : position(_position), rotation(_rotation), scale(_scale)
{

}

glm::quat Transform::euler_degrees_to_quat(const glm::vec3& euler_degrees)
{
  // to_matrix() applies Rx * Ry * Rz, which is exactly glm::eulerAngleXYZ.
  const glm::vec3 r = glm::radians(euler_degrees);
  return glm::normalize(glm::quat_cast(glm::eulerAngleXYZ(r.x, r.y, r.z)));
}

glm::vec3 Transform::quat_to_euler_degrees(const glm::quat& rotation)
{
  float x = 0.0f, y = 0.0f, z = 0.0f;
  glm::extractEulerAngleXYZ(glm::mat4_cast(glm::normalize(rotation)), x, y, z);
  return glm::degrees(glm::vec3(x, y, z));
}

Transform Transform::from_matrix(const glm::mat4& matrix)
{
  glm::vec3 scale_out(1.0f);
  glm::quat rotation_out(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 translation_out(0.0f);
  glm::vec3 skew(0.0f);
  glm::vec4 perspective(0.0f);

  if (!glm::decompose(matrix, scale_out, rotation_out, translation_out, skew, perspective))
  {
    // Degenerate matrix (e.g. zero scale): keep translation only.
    return Transform(glm::vec3(matrix[3]), glm::vec3(0.0f), glm::vec3(0.0f));
  }

  return Transform(translation_out, quat_to_euler_degrees(rotation_out), scale_out);
}

glm::quat Transform::get_rotation_quat() const
{
  return euler_degrees_to_quat(rotation);
}

void Transform::set_rotation_quat(const glm::quat& _rotation)
{
  rotation = quat_to_euler_degrees(_rotation);
}

glm::mat4 Transform::to_matrix() const
{
  glm::mat4 mat = glm::mat4(1.0f);
  mat = glm::translate(mat, position);
  mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  mat = glm::scale(mat, scale);
  return mat;
}

}
