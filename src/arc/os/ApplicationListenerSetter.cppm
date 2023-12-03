module ;

export module OS.ApplicationListenerSetter;

import <GLFW/glfw3.h>;
import <iostream>;
import Core;
import Core.Renderer;
import Core.Input;

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

	inline void setInputMode_Cursor(const CursorMode value, GLFWwindow* window = Core::window) {
		glfwSetInputMode(window, GLFW_CURSOR, static_cast<int>(value));
	}

	inline void setInputMode_StickyKeys(const bool value, GLFWwindow* window = Core::window) {
		glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, value);
	}

	inline void setInputMode_StickyMouseButtons(const bool value, GLFWwindow* window = Core::window) {
		glfwSetInputMode(window, GLFW_STICKY_KEYS, value);
	}

	inline void setInputMode_LockKeyMods(const bool value, GLFWwindow* window = Core::window) {
		glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, value);
	}

	inline void setInputMode_RawMouse(const bool value, GLFWwindow* window = Core::window) {
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, value);
	}
}


inline void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new windowMain dimensions; note that width and
	// height will be significantly larger than specified on retina displays.

	// std::cout << "Resized Buffer: " << width << "|" << height << '\n';

	width = std::max(2, width);
	height = std::max(2, height);

	Core::renderer->resize(width, height);

	if(!Core::currentMonitor) {
		auto& lastBound = Core::lastScreenBound;
		glfwGetWindowPos(window, lastBound.getSrcXRaw(), lastBound.getSrcYRaw());
		glfwGetWindowSize(window, lastBound.getWidthRaw(), lastBound.getHeightRaw());
	}
}

inline void windowRefreshCallback(GLFWwindow* window){

}

inline void mouseBottomCallBack(GLFWwindow* window, const int button, const int action, const int mods){
	// std::cout << button << " | " << action << '\n';
	Core::input->informMouseAction(window, button, action, mods);
}

inline void cursorPosCallback(GLFWwindow* window, const double xPos, const double yPos) {
	Core::input->setPos(static_cast<float>(xPos), static_cast<float>(yPos));
}

inline void dropCallback(GLFWwindow* window, int path_count, const char* paths[]) {


}

inline void cursorEnteredCallback(GLFWwindow* window, const int entered) {
	Core::input->setInbound(entered);
}

inline void charCallback(GLFWwindow* window, unsigned int codepoint) {


}

inline void charModCallback(GLFWwindow* window, unsigned int codepoint, int mods) {


}

inline void scrollCallback(GLFWwindow* window, const double xOffset, const double yOffset){
	Core::input->setScrollOffset(static_cast<float>(xOffset), static_cast<float>(yOffset));
}

inline void scaleCallback(GLFWwindow* window, const float xScale, const float yScale){
	// cout << "Scale CallBack: " << xScale << " | " << yScale << endl;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
inline void keyCallback(GLFWwindow* window, const int key, const int scanCode, const int action, const int mods){
	// std::cout << key << " | " << action << " | " << mods << std::endl;
	if(key >= 0 && key < GLFW_KEY_LAST)Core::input->informKeyAction(window, key, scanCode, action, mods);
}

inline void monitorCallback(GLFWmonitor* monitor, int event){
	Core::currentMonitor = monitor;
}

inline void maximizeCallback(GLFWwindow* window, const int maximized) {
	if(maximized == GLFW_FALSE) {
		const auto& lastBound = Core::lastScreenBound;
		glfwSetWindowPos(window, lastBound.getSrcX(), lastBound.getSrcY());
		glfwSetWindowSize(window, lastBound.getWidth(), lastBound.getHeight());
	}
}

export namespace OS{
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
		// setInputMode_Cursor(OS::CursorMode::hidden);
		// OS::setInputMode_StickyKeys(true);
		// OS::setInputMode_LockKeyMods(true);
	}
}

