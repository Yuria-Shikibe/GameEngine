module;

#include <GLFW/glfw3.h>

// module OS.ApplicationListenerSetter;
module Core.Platform.Current;


import Core;
import GL.Constants;
import UI.Scl;
import std;


void GLFW::charCallback(GLFWwindow* window, unsigned int codepoint){

}

void GLFW::charModCallback(GLFWwindow* window, const unsigned int codepoint, const int mods){
	Core::uiRoot->textInputInform(codepoint, mods);
}

void GLFW::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	if(width * height == 0)return;

	width  = std::max(200, width);
	height = std::max(200, height);

	if(Core::renderer){
		Core::renderer->resize(width, height);
		UI::setScreenSize(Core::renderer->getDrawWidth(), Core::renderer->getDrawHeight());
	}

	if(Core::platform->window->isSmallWindowed()){
		Core::platform->window->informResize(width, height);
	}
}

void GLFW::mouseBottomCallBack(GLFWwindow* window, const int button, const int action, const int mods) {
	// std::cout << button << " | " << action << '\n';
	Core::input.informMouseAction(button, action, mods);
}

void GLFW::cursorPosCallback(GLFWwindow* window, const double xPos, const double yPos) {
	Core::input.cursorMoveInform(static_cast<float>(xPos), static_cast<float>(Core::renderer->getHeight() - yPos));
}

void GLFW::cursorEnteredCallback(GLFWwindow* window, const int entered) {
	Core::input.setInbound(entered);
}

void GLFW::scrollCallback(GLFWwindow* window, const double xOffset, const double yOffset) {
	Core::input.setScrollOffset(static_cast<float>(xOffset), static_cast<float>(yOffset));
}

void GLFW::keyCallback(GLFWwindow* window, const int key, const int scanCode, const int action, const int mods) {
	// std::cout << key << " | " << action << " | " << mods << std::endl;
	if(key >= 0 && key < GLFW_KEY_LAST)Core::input.informKeyAction(key, scanCode, action, mods);
}

void GLFW::monitorCallback(GLFWmonitor* monitor, int event) {
	//Core::platform->window->resetMonitor(monitor);
}

void GLFW::maximizeCallback(GLFWwindow* window, const int maximized) {
	if(maximized == GLFW_FALSE) {
		Core::platform->window->quitMaximumizedWindow();
	}
}

void GLFW::winPosCallBack(GLFWwindow* window, const int xpos, const int ypos) {
	if(Core::platform->window->isSmallWindowed()){
		Core::platform->window->informMove(xpos, ypos);
	}
}
