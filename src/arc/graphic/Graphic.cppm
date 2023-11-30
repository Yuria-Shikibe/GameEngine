module ;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

export module Graphic;

import GL.GL_Exception;
import <string>;

export namespace Graphic{

	inline void enableMSAA(const unsigned int scale = 2){
		glfwWindowHint(GLFW_SAMPLES, scale);
	}

	// ReSharper disable once CppInconsistentNaming
	inline void initGLFW(){
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		// glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
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
		if (windowMain == nullptr)
		{
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

		glViewport(0, 0, scr_width, scr_height);
		glfwSwapInterval(1);
		glEnable(GL_MULTISAMPLE);

//		initRenderer(scr_width, scr_height);

		return windowMain;
	}
}
