#pragma once

#include "calgine/core/renderer/vertex.h"
#include "calgine/core/renderer/mesh.h"
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <cmath>

namespace Calgine::Primitives {

// Helper function to create a Mesh from vertices and indices
inline Mesh create_mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
  return Mesh(std::span<const Vertex>(vertices), std::span<const uint32_t>(indices));
}

inline Mesh cube(const glm::vec4& colour = glm::vec4(1.0f)) {
  std::vector<Vertex> vertices = {
    // Front face (z = 0.5)
    {{colour}, {-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
    {{colour}, { 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
    {{colour}, { 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
    {{colour}, {-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},

    // Back face (z = -0.5)
    {{colour}, { 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
    {{colour}, {-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
    {{colour}, {-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
    {{colour}, { 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},

    // Top face (y = 0.5)
    {{colour}, {-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
    {{colour}, { 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
    {{colour}, { 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
    {{colour}, {-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},

    // Bottom face (y = -0.5)
    {{colour}, {-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
    {{colour}, { 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
    {{colour}, { 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
    {{colour}, {-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},

    // Right face (x = 0.5)
    {{colour}, { 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
    {{colour}, { 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
    {{colour}, { 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
    {{colour}, { 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},

    // Left face (x = -0.5)
    {{colour}, {-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
    {{colour}, {-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
    {{colour}, {-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
    {{colour}, {-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
  };

  std::vector<uint32_t> indices = {
    0, 1, 2, 0, 2, 3,       // Front
    4, 5, 6, 4, 6, 7,       // Back
    8, 9, 10, 8, 10, 11,    // Top
    12, 13, 14, 12, 14, 15, // Bottom
    16, 17, 18, 16, 18, 19, // Right
    20, 21, 22, 20, 22, 23  // Left
  };

  return create_mesh(vertices, indices);
}

inline Mesh sphere(float radius = 0.5f, uint32_t segments = 32, uint32_t rings = 16, const glm::vec4& colour = glm::vec4(1.0f)) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  const float pi = 3.14159265359f;

  // Generate vertices
  for (uint32_t r = 0; r <= rings; ++r) {
    float phi = pi * r / rings;
    float sin_phi = std::sin(phi);
    float cos_phi = std::cos(phi);

    for (uint32_t s = 0; s <= segments; ++s) {
      float theta = 2.0f * pi * s / segments;
      float sin_theta = std::sin(theta);
      float cos_theta = std::cos(theta);

      glm::vec3 position = radius * glm::vec3(
        sin_phi * cos_theta,
        cos_phi,
        sin_phi * sin_theta
      );

      glm::vec3 normal = glm::normalize(position);
      glm::vec2 uv(static_cast<float>(s) / segments, static_cast<float>(r) / rings);

      vertices.push_back({colour, position, normal, uv});
    }
  }

  // Generate indices
  for (uint32_t r = 0; r < rings; ++r) {
    for (uint32_t s = 0; s < segments; ++s) {
      uint32_t a = r * (segments + 1) + s;
      uint32_t b = a + segments + 1;

      indices.push_back(a);
      indices.push_back(b);
      indices.push_back(a + 1);

      indices.push_back(b);
      indices.push_back(b + 1);
      indices.push_back(a + 1);
    }
  }

  return create_mesh(vertices, indices);
}

// ============================================================================
// PYRAMID (Square-based)
// ============================================================================
inline Mesh pyramid(const glm::vec4& colour = glm::vec4(1.0f)) {
  std::vector<Vertex> vertices = {
    // Base vertices
    {{colour}, {-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
    {{colour}, { 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
    {{colour}, { 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
    {{colour}, {-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},

    // Apex
    {{colour}, { 0.0f,  0.5f,  0.0f}, { 0.0f,  0.707f,  0.707f}, {0.5f, 1.0f}},
  };

  std::vector<uint32_t> indices = {
    // Base
    0, 1, 2,
    0, 2, 3,

    // Front face
    0, 4, 1,

    // Right face
    1, 4, 2,

    // Back face
    2, 4, 3,

    // Left face
    3, 4, 0,
  };

  return create_mesh(vertices, indices);
}

// ============================================================================
// TRIANGULAR PRISM
// ============================================================================
inline Mesh triangular_prism(float height = 1.0f, const glm::vec4& colour = glm::vec4(1.0f)) {
  float h2 = height * 0.5f;
  float r = 0.5f;

  std::vector<Vertex> vertices = {
    // Front triangle
    {{colour}, {-r, 0.0f,  h2}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
    {{colour}, { r, 0.0f,  h2}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
    {{colour}, { 0.0f,  r,  h2}, { 0.0f,  0.0f,  1.0f}, {0.5f, 1.0f}},

    // Back triangle
    {{colour}, {-r, 0.0f, -h2}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
    {{colour}, { 0.0f,  r, -h2}, { 0.0f,  0.0f, -1.0f}, {0.5f, 1.0f}},
    {{colour}, { r, 0.0f, -h2}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},

    // Bottom face vertices
    {{colour}, {-r, 0.0f,  h2}, {-0.707f, -0.707f,  0.0f}, {0.0f, 1.0f}},
    {{colour}, { r, 0.0f,  h2}, {-0.707f, -0.707f,  0.0f}, {1.0f, 1.0f}},
    {{colour}, { r, 0.0f, -h2}, {-0.707f, -0.707f,  0.0f}, {1.0f, 0.0f}},
    {{colour}, {-r, 0.0f, -h2}, {-0.707f, -0.707f,  0.0f}, {0.0f, 0.0f}},

    // Slant face 1 vertices
    {{colour}, { 0.0f,  r,  h2}, { 0.707f,  0.707f,  0.0f}, {0.0f, 1.0f}},
    {{colour}, { r, 0.0f,  h2}, { 0.707f,  0.707f,  0.0f}, {1.0f, 1.0f}},
    {{colour}, { r, 0.0f, -h2}, { 0.707f,  0.707f,  0.0f}, {1.0f, 0.0f}},
    {{colour}, { 0.0f,  r, -h2}, { 0.707f,  0.707f,  0.0f}, {0.0f, 0.0f}},

    // Slant face 2 vertices
    {{colour}, {-r, 0.0f,  h2}, {-0.707f,  0.707f,  0.0f}, {0.0f, 1.0f}},
    {{colour}, { 0.0f,  r,  h2}, {-0.707f,  0.707f,  0.0f}, {1.0f, 1.0f}},
    {{colour}, { 0.0f,  r, -h2}, {-0.707f,  0.707f,  0.0f}, {1.0f, 0.0f}},
    {{colour}, {-r, 0.0f, -h2}, {-0.707f,  0.707f,  0.0f}, {0.0f, 0.0f}},
  };

  std::vector<uint32_t> indices = {
    // Front triangle
    0, 1, 2,

    // Back triangle
    3, 5, 4,

    // Bottom face
    6, 7, 8, 6, 8, 9,

    // Slant face 1
    10, 11, 12, 10, 12, 13,

    // Slant face 2
    14, 15, 16, 14, 16, 17,
  };

  return create_mesh(vertices, indices);
}

// ============================================================================
// CYLINDER
// ============================================================================
inline Mesh cylinder(float radius = 0.5f, float height = 1.0f, uint32_t segments = 32, const glm::vec4& colour = glm::vec4(1.0f)) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  const float pi = 3.14159265359f;
  float h2 = height * 0.5f;

  // Top cap center
  uint32_t top_center = vertices.size();
  vertices.push_back({colour, {0.0f, h2, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f}});

  // Top cap vertices
  uint32_t top_start = vertices.size();
  for (uint32_t i = 0; i <= segments; ++i) {
    float angle = 2.0f * pi * i / segments;
    float x = radius * std::cos(angle);
    float z = radius * std::sin(angle);
    vertices.push_back({colour, {x, h2, z}, {0.0f, 1.0f, 0.0f}, {0.5f + 0.5f * std::cos(angle), 0.5f + 0.5f * std::sin(angle)}});
  }

  // Bottom cap center
  uint32_t bottom_center = vertices.size();
  vertices.push_back({colour, {0.0f, -h2, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}});

  // Bottom cap vertices
  uint32_t bottom_start = vertices.size();
  for (uint32_t i = 0; i <= segments; ++i) {
    float angle = 2.0f * pi * i / segments;
    float x = radius * std::cos(angle);
    float z = radius * std::sin(angle);
    vertices.push_back({colour, {x, -h2, z}, {0.0f, -1.0f, 0.0f}, {0.5f + 0.5f * std::cos(angle), 0.5f - 0.5f * std::sin(angle)}});
  }

  // Side vertices
  uint32_t side_top_start = vertices.size();
  for (uint32_t i = 0; i <= segments; ++i) {
    float angle = 2.0f * pi * i / segments;
    float x = radius * std::cos(angle);
    float z = radius * std::sin(angle);
    float normal_x = std::cos(angle);
    float normal_z = std::sin(angle);
    vertices.push_back({colour, {x, h2, z}, {normal_x, 0.0f, normal_z}, {static_cast<float>(i) / segments, 1.0f}});
  }

  uint32_t side_bottom_start = vertices.size();
  for (uint32_t i = 0; i <= segments; ++i) {
    float angle = 2.0f * pi * i / segments;
    float x = radius * std::cos(angle);
    float z = radius * std::sin(angle);
    float normal_x = std::cos(angle);
    float normal_z = std::sin(angle);
    vertices.push_back({colour, {x, -h2, z}, {normal_x, 0.0f, normal_z}, {static_cast<float>(i) / segments, 0.0f}});
  }

  // Top cap triangles
  for (uint32_t i = 0; i < segments; ++i) {
    indices.push_back(top_center);
    indices.push_back(top_start + i);
    indices.push_back(top_start + i + 1);
  }

  // Bottom cap triangles
  for (uint32_t i = 0; i < segments; ++i) {
    indices.push_back(bottom_center);
    indices.push_back(bottom_start + i + 1);
    indices.push_back(bottom_start + i);
  }

  // Side quads
  for (uint32_t i = 0; i < segments; ++i) {
    uint32_t a = side_top_start + i;
    uint32_t b = side_bottom_start + i;
    indices.push_back(a);
    indices.push_back(b);
    indices.push_back(a + 1);
    indices.push_back(b);
    indices.push_back(b + 1);
    indices.push_back(a + 1);
  }

  return create_mesh(vertices, indices);
}

// ============================================================================
// CONE
// ============================================================================
inline Mesh cone(float radius = 0.5f, float height = 1.0f, uint32_t segments = 32, const glm::vec4& colour = glm::vec4(1.0f)) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  const float pi = 3.14159265359f;
  float h2 = height * 0.5f;

  // Base center
  uint32_t base_center = vertices.size();
  vertices.push_back({colour, {0.0f, -h2, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}});

  // Base vertices
  uint32_t base_start = vertices.size();
  for (uint32_t i = 0; i <= segments; ++i) {
    float angle = 2.0f * pi * i / segments;
    float x = radius * std::cos(angle);
    float z = radius * std::sin(angle);
    vertices.push_back({colour, {x, -h2, z}, {0.0f, -1.0f, 0.0f}, {0.5f + 0.5f * std::cos(angle), 0.5f + 0.5f * std::sin(angle)}});
  }

  // Apex
  uint32_t apex = vertices.size();
  vertices.push_back({colour, {0.0f, h2, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}});

  // Side vertices
  uint32_t side_base_start = vertices.size();
  for (uint32_t i = 0; i <= segments; ++i) {
    float angle = 2.0f * pi * i / segments;
    float x = radius * std::cos(angle);
    float z = radius * std::sin(angle);
    float normal_x = std::cos(angle) * height;
    float normal_z = std::sin(angle) * height;
    glm::vec3 normal = glm::normalize(glm::vec3(normal_x, radius, normal_z));
    vertices.push_back({colour, {x, -h2, z}, normal, {static_cast<float>(i) / segments, 0.0f}});
  }

  // Base cap triangles
  for (uint32_t i = 0; i < segments; ++i) {
    indices.push_back(base_center);
    indices.push_back(base_start + i + 1);
    indices.push_back(base_start + i);
  }

  // Side triangles
  for (uint32_t i = 0; i < segments; ++i) {
    indices.push_back(side_base_start + i);
    indices.push_back(apex);
    indices.push_back(side_base_start + i + 1);
  }

  return create_mesh(vertices, indices);
}

// ============================================================================
// TORUS (Doughnut)
// ============================================================================
inline Mesh torus(float major_radius = 0.5f, float minor_radius = 0.2f, uint32_t major_segments = 32, uint32_t minor_segments = 16, const glm::vec4& colour = glm::vec4(1.0f)) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  const float pi = 3.14159265359f;

  // Generate vertices
  for (uint32_t i = 0; i <= major_segments; ++i) {
    float theta = 2.0f * pi * i / major_segments;
    float cos_theta = std::cos(theta);
    float sin_theta = std::sin(theta);

    for (uint32_t j = 0; j <= minor_segments; ++j) {
      float phi = 2.0f * pi * j / minor_segments;
      float cos_phi = std::cos(phi);
      float sin_phi = std::sin(phi);

      float x = (major_radius + minor_radius * cos_phi) * cos_theta;
      float y = minor_radius * sin_phi;
      float z = (major_radius + minor_radius * cos_phi) * sin_theta;

      glm::vec3 position(x, y, z);
      glm::vec3 normal(cos_phi * cos_theta, sin_phi, cos_phi * sin_theta);

      glm::vec2 uv(static_cast<float>(i) / major_segments, static_cast<float>(j) / minor_segments);

      vertices.push_back({colour, position, normal, uv});
    }
  }

  // Generate indices
  for (uint32_t i = 0; i < major_segments; ++i) {
    for (uint32_t j = 0; j < minor_segments; ++j) {
      uint32_t a = i * (minor_segments + 1) + j;
      uint32_t b = a + minor_segments + 1;

      indices.push_back(a);
      indices.push_back(b);
      indices.push_back(a + 1);

      indices.push_back(b);
      indices.push_back(b + 1);
      indices.push_back(a + 1);
    }
  }

  return create_mesh(vertices, indices);
}

inline Mesh icosahedron(const glm::vec4& colour = glm::vec4(1.0f)) {
  const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;
  const float inv_len = 1.0f / std::sqrt(phi * phi + 1.0f);

  std::vector<Vertex> vertices = {
    {{colour}, {-1.0f,  phi, 0.0f} * inv_len, glm::normalize(glm::vec3(-1.0f,  phi, 0.0f)), {0.0f, 0.0f}},
    {{colour}, { 1.0f,  phi, 0.0f} * inv_len, glm::normalize(glm::vec3( 1.0f,  phi, 0.0f)), {1.0f, 0.0f}},
    {{colour}, {-1.0f, -phi, 0.0f} * inv_len, glm::normalize(glm::vec3(-1.0f, -phi, 0.0f)), {0.0f, 1.0f}},
    {{colour}, { 1.0f, -phi, 0.0f} * inv_len, glm::normalize(glm::vec3( 1.0f, -phi, 0.0f)), {1.0f, 1.0f}},

    {{colour}, { 0.0f, -1.0f,  phi} * inv_len, glm::normalize(glm::vec3( 0.0f, -1.0f,  phi)), {0.0f, 0.0f}},
    {{colour}, { 0.0f,  1.0f,  phi} * inv_len, glm::normalize(glm::vec3( 0.0f,  1.0f,  phi)), {1.0f, 0.0f}},
    {{colour}, { 0.0f, -1.0f, -phi} * inv_len, glm::normalize(glm::vec3( 0.0f, -1.0f, -phi)), {0.0f, 1.0f}},
    {{colour}, { 0.0f,  1.0f, -phi} * inv_len, glm::normalize(glm::vec3( 0.0f,  1.0f, -phi)), {1.0f, 1.0f}},

    {{colour}, { phi, 0.0f, -1.0f} * inv_len, glm::normalize(glm::vec3( phi, 0.0f, -1.0f)), {0.0f, 0.0f}},
    {{colour}, { phi, 0.0f,  1.0f} * inv_len, glm::normalize(glm::vec3( phi, 0.0f,  1.0f)), {1.0f, 0.0f}},
    {{colour}, {-phi, 0.0f, -1.0f} * inv_len, glm::normalize(glm::vec3(-phi, 0.0f, -1.0f)), {0.0f, 1.0f}},
    {{colour}, {-phi, 0.0f,  1.0f} * inv_len, glm::normalize(glm::vec3(-phi, 0.0f,  1.0f)), {1.0f, 1.0f}},
  };

  std::vector<uint32_t> indices = {
    0, 11, 5,   0, 5, 1,    0, 1, 7,   0, 7, 10,  0, 10, 11,
    1, 5, 9,    5, 11, 4,   11, 10, 2, 10, 7, 6,  7, 1, 8,
    3, 9, 4,    3, 4, 2,    3, 2, 6,   3, 6, 8,   3, 8, 9,
    4, 9, 5,    2, 4, 11,   6, 2, 10,  8, 6, 7,   9, 8, 1,
  };

  return create_mesh(vertices, indices);
}

inline Mesh quad(float width = 1.0f, float height = 1.0f, const glm::vec4& colour = glm::vec4(1.0f)) {
  float w2 = width * 0.5f;
  float h2 = height * 0.5f;

  std::vector<Vertex> vertices = {
    {{colour}, {-w2, 0.0f,  h2}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    {{colour}, { w2, 0.0f,  h2}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{colour}, { w2, 0.0f, -h2}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{colour}, {-w2, 0.0f, -h2}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
  };

  std::vector<uint32_t> indices = {
    0, 1, 2,
    0, 2, 3,
  };

  return create_mesh(vertices, indices);
}

}
