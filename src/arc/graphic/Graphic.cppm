module ;

export module Graphic;

import <glad/glad.h>;
import <GLFW/glfw3.h>;
import Geom.Shape.Rect_Orthogonal;
import RuntimeException;
import std;

namespace Graphic{
	void glDebugCallback(GLenum source, GLenum type, const GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
	{
		const std::string_view src_str = [source]{
			switch (source)
			{
				case GL_DEBUG_SOURCE_API: return "API";
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
				case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
				case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
				case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
				case GL_DEBUG_SOURCE_OTHER: return "OTHER";
				default: return "UNKNOWN";
			}
		}();

		const std::string_view type_str = [type]() {
			switch (type)
			{
				case GL_DEBUG_TYPE_ERROR: return "ERROR";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
				case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
				case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
				case GL_DEBUG_TYPE_MARKER: return "MARKER";
				case GL_DEBUG_TYPE_OTHER: return "OTHER";
				default: return "UNKNOWN";
			}
		}();

		const std::string_view severity_str = [severity]{
			switch (severity) {
				case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
				case GL_DEBUG_SEVERITY_LOW: return "LOW";
				case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
				case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
				default: return "UNKNOWN";
			}
		}();

		std::println(std::cout, "[GL {}-{}-{} | {}]: {}", src_str, type_str, severity_str, id, message);
	}

	inline void throw_GL_Exception(const int error_code, const char* description) {
		throw ext::IllegalArguments{"ERROR CODE: " + std::to_string(error_code) + "\n\n" + std::string(description) + "\n\n"};
	}
}



export namespace Graphic{
	void enableMSAA(const int scale = 2){
		glfwWindowHint(GLFW_SAMPLES, scale);
	}

	void setupGLDebug(){
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(glDebugCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	}

	void initOpenGL(){
		glfwSetErrorCallback(throw_GL_Exception);
		if(!glfwInit()) {
			throw ext::RuntimeException{"Unable to initialize GLFW!"};
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	const GLFWvidmode* getVideoMode(GLFWmonitor* const monitor = glfwGetPrimaryMonitor()) {
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		return mode;
	}

	/**
	 *  glfw: whenever the windowMain size changed (by OS or user resize) this callback function executes
	 */
	inline void processInput(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)glfwSetWindowShouldClose(window, true);
	}

	inline GLFWwindow* initWindow(const std::string& windowTitle, const int scr_width = 800, const int scr_height = 800, GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr) noexcept(false) {
		// glfw windowMain creation
		GLFWwindow* windowMain = glfwCreateWindow(scr_width, scr_height, windowTitle.c_str(), monitor, share);
		// renderer = new Renderer();
		if (windowMain == nullptr){
			glfwTerminate();
			throw ext::RuntimeException("Failed to create GLFW windowMain");
		}

		glfwMakeContextCurrent(windowMain);
		// glfwSetFramebufferSizeCallback(windowMain, framebufferSizeCallback);
		// glfwSetInputMode(windowMain, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


		// glad: load all OpenGL function pointers
		// ---------------------------------------
		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
		{
			throw ext::RuntimeException("Failed to create GLFW windowMain");
		}

		setupGLDebug();

		return windowMain;
	}

	void fullScreen(GLFWwindow* window, GLFWmonitor*& monitor, const std::string& windowTitle) {
		monitor = glfwGetPrimaryMonitor();
		const auto mode = getVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		//RESIZE RENDERER!!
	}

	void windowize(GLFWwindow* window, GLFWmonitor*& monitor, const Geom::Shape::OrthoRectInt& lastBound, const std::string& windowTitle) {
		const auto mode = getVideoMode(monitor);
		monitor = nullptr;
		glfwSetWindowMonitor(window, monitor, lastBound.getSrcX(), lastBound.getSrcY(), lastBound.getWidth(), lastBound.getHeight(), mode->refreshRate);
		//RESIZE RENDERER!!
	}
}
