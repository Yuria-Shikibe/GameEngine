module;

module OS.ApplicationListenerSetter;

import Core;
import GL.Constants;

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	if(width * height == 0)return;

	if(Core::windowized) {
		Core::windowized = false;
		width            = Core::lastScreenBound.getWidth();
		height           = Core::lastScreenBound.getHeight();
		glfwSetWindowSize(window, width, height);
	}

	std::cout << "Resized Buffer: " << width << "|" << height << std::endl;

	width  = std::max(200, width);
	height = std::max(200, height);

	Core::renderer->resize(width, height);

	if(!Core::currentMonitor && !Core::maximized) {
		Core::lastScreenBound.setSize(width, height);
	}
}

void mouseBottomCallBack(GLFWwindow* window, const int button, const int action, const int mods) {
	// std::cout << button << " | " << action << '\n';
	Core::input->informMouseAction(window, button, action, mods);
}

void cursorPosCallback(GLFWwindow* window, const double xPos, const double yPos) {
	Core::input->cursorMoveInform(static_cast<float>(xPos), static_cast<float>(Core::renderer->getHeight() - yPos));
}

void cursorEnteredCallback(GLFWwindow* window, const int entered) {
	Core::input->setInbound(entered);
}

void scrollCallback(GLFWwindow* window, const double xOffset, const double yOffset) {
	Core::input->setScrollOffset(static_cast<float>(xOffset), static_cast<float>(yOffset));
}

void keyCallback(GLFWwindow* window, const int key, const int scanCode, const int action, const int mods) {
	// std::cout << key << " | " << action << " | " << mods << std::endl;
	if(key >= 0 && key < GLFW_KEY_LAST)Core::input->informKeyAction(window, key, scanCode, action, mods);
}

void monitorCallback(GLFWmonitor* monitor, int event) {
	Core::currentMonitor = monitor;
}

void maximizeCallback(GLFWwindow* window, const int maximized) {
	if(maximized == GLFW_FALSE) {
		const auto& lastBound = Core::lastScreenBound;
		glfwSetWindowPos(window, lastBound.getSrcX(), lastBound.getSrcY());
	}

	Core::windowized = !static_cast<bool>(maximized);
}

void winPosCallBack(GLFWwindow* window, int xpos, int ypos) {
	if(!Core::currentMonitor && !Core::maximized) {
		Core::lastScreenBound.setSrc(xpos, ypos);
	}
}
