#include "shader.h"

#include <cassert>
#include <glad/gl.h>
#include "../log.h"

namespace Calgine{

Shader::Shader(const std::string& vertex_source, const std::string& fragment_source)
{
  // Create an empty vertex shader handle
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

  // Send the vertex shader source code to GL
  // Note that std::string's .c_str is NULL character terminated.
  const GLchar* source = vertex_source.c_str();
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
  source = (const GLchar *)fragment_source.c_str();
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

void Shader::bind()
{
	assert(renderer_id != 0 && "Renderer ID is Null!");
	glUseProgram(renderer_id);
}

void Shader::unbind()  
{
	glUseProgram(0);
}

}