#include "shader.h"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include "../log.h"

namespace Calgine{

Shader::Shader(const ShaderProgram& program)
{
	// Code modified from https://wikis.khronos.org/opengl/Shader_Compilation for use in Calgine.

  // Create an empty vertex shader handle
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

  // Send the vertex shader source code to GL
  // Note that std::string's .c_str is NULL character terminated.
  const GLchar* source = program.vertex.c_str();
  glShaderSource(vertexShader, 1, &source, 0);

  // Compile the vertex shader
  glCompileShader(vertexShader);

  GLint isCompiled = 0;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE)
  {
  	GLint maxLength = 0;
  	glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

  	// The maxLength includes the NULL character
  	std::vector<GLchar> infoLog(maxLength);
  	glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
  
  	// We don't need the shader anymore.
  	glDeleteShader(vertexShader);

  	// Use the infoLog as you see fit.
    Log::get_engine_logger()->error("Vertex Shader Compilation Error! GL Info Log:");
    Log::get_engine_logger()->error("{}", infoLog.data());
      
  	return;
  }

  // Create an empty fragment shader handle
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

  // Send the fragment shader source code to GL
  // Note that std::string's .c_str is NULL character terminated.
  source = (const GLchar *)program.fragment.c_str();
  glShaderSource(fragmentShader, 1, &source, 0);

  // Compile the fragment shader
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE)
  {
  	GLint maxLength = 0;
  	glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

  	// The maxLength includes the NULL character
  	std::vector<GLchar> infoLog(maxLength);
  	glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
  
  	// We don't need the shader anymore.
  	glDeleteShader(fragmentShader);
  	// Either of them. Don't leak shaders.
  	glDeleteShader(vertexShader);

		Log::get_engine_logger()->error("Fragment Shader Compilation Error! GL Info Log:");
    Log::get_engine_logger()->error("{}", infoLog.data());

  	return;
	}

	// Vertex and fragment shaders are successfully compiled.
	// Now time to link them together into a program.
	// Get a program object.
	renderer_id = glCreateProgram();

	// Attach our shaders to our program
	glAttachShader(renderer_id, vertexShader);
	glAttachShader(renderer_id, fragmentShader);

	// Link our program
	glLinkProgram(renderer_id);

	// Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(renderer_id, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(renderer_id, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(renderer_id, maxLength, &maxLength, &infoLog[0]);

		// We don't need the program anymore.
		glDeleteProgram(renderer_id);
		// Don't leak shaders either.
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		Log::get_engine_logger()->error("Shader Link Error! GL Info Log:");
    Log::get_engine_logger()->error("{}", infoLog.data());

		// In this simple program, we'll just leave
		return;
	}

	// Always detach shaders after a successful link.
	glDetachShader(renderer_id, vertexShader);
	glDetachShader(renderer_id, fragmentShader);
}

Shader::~Shader()
{
	glDeleteProgram(renderer_id);
}

void Shader::bind() const
{
	assert(renderer_id != 0 && "Renderer ID is Null!");
	glUseProgram(renderer_id);
}

void Shader::unbind() const
{
	glUseProgram(0);
}

GLint Shader::get_uniform_location(const std::string& name) const
{
	if (uniform_cache.find(name) != uniform_cache.end())
		return uniform_cache[name];

	GLint location = glGetUniformLocation(renderer_id, name.c_str());
	if (location == -1)
		Log::get_engine_logger()->warn("Uniform '{}' not found in shader", name);

	uniform_cache[name] = location;
	return location;
}

void Shader::set_uniform_1i(const std::string& name, int value) const
{
	glUniform1i(get_uniform_location(name), value);
}

void Shader::set_uniform_1f(const std::string& name, float value) const 
{
	glUniform1f(get_uniform_location(name), value);
}

void Shader::set_uniform_2f(const std::string& name, float v0, float v1) const 
{
	glUniform2f(get_uniform_location(name), v0, v1);
}

void Shader::set_uniform_3f(const std::string& name, float v0, float v1, float v2) const
{
	glUniform3f(get_uniform_location(name), v0, v1, v2);
}

void Shader::set_uniform_4f(const std::string& name, float v0, float v1, float v2, float v3) const
{
	glUniform4f(get_uniform_location(name), v0, v1, v2, v3);
}

void Shader::set_uniform_mat4(const std::string& name, const glm::mat4& matrix) const
{
	glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

}