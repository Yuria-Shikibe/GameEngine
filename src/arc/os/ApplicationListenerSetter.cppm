module;

#include <GLFW/glfw3.h>

export module OS.ApplicationListenerSetter;

import std;

/**
*If the mode is `GLFW_CURSOR`, the value must be one of the following cursor
* modes:
*-`GLFW_CURSOR_NORMAL` makes the cursor visible and behaving normally.
* -`GLFW_CURSOR_HIDDEN` makes the cursor invisible when it is over the
* content area of the window but does not restrict the cursor from leaving.
* -`GLFW_CURSOR_DISABLED` hides and grabs the cursor, providing virtual
* and unlimited cursor movement.This is useful for implementing for
* example 3D camera controls.
*
*If the mode is `GLFW_STICKY_KEYS`, the value must be either `GLFW_TRUE` to
* enable sticky keys, or `GLFW_FALSE` to disable it.If sticky keys are
* enabled, a key press will ensure that @ref glfwGetKey returns `GLFW_PRESS`
* the next time it is called even if the key had been released before the
* call.This is useful when you are only interested in whether keys have been
* pressed but not when or in which order.
*
*If the mode is `GLFW_STICKY_MOUSE_BUTTONS`, the value must be either
* `GLFW_TRUE` to enable sticky mouse buttons, or `GLFW_FALSE` to disable it.
* If sticky mouse buttons are enabled, a mouse button press will ensure that
* @ref glfwGetMouseButton returns `GLFW_PRESS` the next time it is called even
* if the mouse button had been released before the call.This is useful when
* you are only interested in whether mouse buttons have been pressed but not
*when or in which order.
*
*If the mode is `GLFW_LOCK_KEY_MODS`, the value must be either `GLFW_TRUE` to
* enable lock key modifier bits, or `GLFW_FALSE` to disable them.If enabled,
* callbacks that receive modifier bits will also have the @ref
* GLFW_MOD_CAPS_LOCK bit set when the event was generated with Caps Lock on,
* and the @ref GLFW_MOD_NUM_LOCK bit when Num Lock was on.
*
* If the mode is `GLFW_RAW_MOUSE_MOTION`, the value must be either `GLFW_TRUE`
* to enable raw(unscaled and unaccelerated) mouse motion when the cursor is
* disabled, or `GLFW_FALSE` to disable it.If raw motion is not supported,
* attempting to set this will emit @ref GLFW_PLATFORM_ERROR.Call @ref
* glfwRawMouseMotionSupported to check for support.
*/

export namespace OS{
	enum class CursorMode{
		normal = GLFW_CURSOR_NORMAL,
		hidden = GLFW_CURSOR_HIDDEN,
		disabled = GLFW_CURSOR_DISABLED
	};

	void setInputMode_Cursor(const CursorMode value, GLFWwindow* window) {
		glfwSetInputMode(window, GLFW_CURSOR, static_cast<int>(value));
	}

	void setInputMode_StickyKeys(const bool value, GLFWwindow* window) {
		glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, value);
	}

	void setInputMode_StickyMouseButtons(const bool value, GLFWwindow* window) {
		glfwSetInputMode(window, GLFW_STICKY_KEYS, value);
	}

	void setInputMode_LockKeyMods(const bool value, GLFWwindow* window) {
		glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, value);
	}

	void setInputMode_RawMouse(const bool value, GLFWwindow* window) {
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, value);
	}
}


void windowRefreshCallback(GLFWwindow* window){

}

void dropCallback(GLFWwindow* window, int path_count, const char* paths[]) {

}

void charCallback(GLFWwindow* window, unsigned int codepoint);

void charModCallback(GLFWwindow* window, unsigned int codepoint, int mods);

void scaleCallback(GLFWwindow* window, const float xScale, const float yScale){
	// cout << "Scale CallBack: " << xScale << " | " << yScale << endl;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height);

void mouseBottomCallBack(GLFWwindow* window, int button, int action, int mods);

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos);

void cursorEnteredCallback(GLFWwindow* window, int entered);

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods);

void monitorCallback(GLFWmonitor* monitor, int event);

void maximizeCallback(GLFWwindow* window, int maximized);

void winPosCallBack(GLFWwindow* window, int xpos, int ypos);

export namespace OS{
	inline void setApplicationIcon(GLFWwindow* window, const GLFWimage* image) {
		glfwSetWindowIcon(window, 1, image);
	}

	void loadListeners(GLFWwindow* window) {
		glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
		glfwSetCursorPosCallback(window, cursorPosCallback);
		glfwSetDropCallback(window, dropCallback);
		glfwSetCursorEnterCallback(window, cursorEnteredCallback);
		glfwSetCharCallback(window, charCallback);
		glfwSetCharModsCallback(window, charModCallback);

		glfwSetKeyCallback(window, keyCallback);
		glfwSetMonitorCallback(monitorCallback);

		glfwSetScrollCallback(window, scrollCallback);
		glfwSetWindowContentScaleCallback(window, scaleCallback);

		glfwSetWindowMaximizeCallback(window, maximizeCallback);
		glfwSetWindowRefreshCallback(window, windowRefreshCallback);
		glfwSetMouseButtonCallback(window, mouseBottomCallBack);
		glfwSetWindowPosCallback(window, winPosCallBack);
		// glfwSetWindowSizeCallback(window, framebufferSizeCallback);
		// glfwMini
		// setInputMode_Cursor(OS::CursorMode::hidden, window);
		// OS::setInputMode_StickyKeys(true);
		// OS::setInputMode_LockKeyMods(true);
	}
}

