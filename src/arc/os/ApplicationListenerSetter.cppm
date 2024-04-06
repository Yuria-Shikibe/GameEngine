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

	void setInputMode_Cursor(const CursorMode value, void* window) {
		glfwSetInputMode(static_cast<GLFWwindow*>(window), GLFW_CURSOR, static_cast<int>(value));
	}

	void setInputMode_StickyKeys(const bool value, void* window) {
		glfwSetInputMode(static_cast<GLFWwindow*>(window), GLFW_STICKY_MOUSE_BUTTONS, value);
	}

	void setInputMode_StickyMouseButtons(const bool value, void* window) {
		glfwSetInputMode(static_cast<GLFWwindow*>(window), GLFW_STICKY_KEYS, value);
	}

	void setInputMode_LockKeyMods(const bool value, void* window) {
		glfwSetInputMode(static_cast<GLFWwindow*>(window), GLFW_LOCK_KEY_MODS, value);
	}

	void setInputMode_RawMouse(const bool value, GLFWwindow* window) {
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, value);
	}
}
