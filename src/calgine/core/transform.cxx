#include "transform.h"
#include "glm/ext/vector_float3.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Calgine {

Transform::Transform(const glm::vec3 _position, const glm::vec3 _rotation, const glm::vec3 _scale)
  : position(_position), rotation(_rotation), scale(_scale)
{

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