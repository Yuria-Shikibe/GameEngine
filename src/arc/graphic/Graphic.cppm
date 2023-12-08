module ;

export module Graphic;

import <glad/glad.h>;
import <GLFW/glfw3.h>;
import Geom.Shape.Rect_Orthogonal;
import GL.GL_Exception;
import RuntimeException;
import <string>;
import <sstream>;

inline void throw_GL_Exception(const int error_code, const char* description) {
	throw ext::IllegalArguments{"ERROR CODE: " + std::to_string(error_code) + "\n\n" + std::string(description) + "\n\n"};
}

export namespace Graphic{
	inline void enableMSAA(const int scale = 2){
		glfwWindowHint(GLFW_SAMPLES, scale);
	}

	// ReSharper disable once CppInconsistentNaming
	inline void initOpenGL(){
		glfwSetErrorCallback(throw_GL_Exception);
		glfwInit();
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
			throw GL_Exception("Failed to create GLFW windowMain");
		}

		glfwMakeContextCurrent(windowMain);
		// glfwSetFramebufferSizeCallback(windowMain, framebufferSizeCallback);
		// glfwSetInputMode(windowMain, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// glad: load all OpenGL function pointers
		// ---------------------------------------
		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
		{
			throw GL_Exception("Failed to create GLFW windowMain");
		}

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
