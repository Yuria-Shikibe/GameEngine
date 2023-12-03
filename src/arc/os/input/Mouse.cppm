//
// Created by Matrix on 2023/11/19.
//

module ;

#include <GLFW/glfw3.h>

export module OS.Mouse;

import <functional>;

export namespace OS{
	struct MouseBind {
	protected:
		int button = 0;
		int expectedState = 1;

		std::function<void()> action = nullptr;

		int mod = 0;

	public:
		MouseBind(const int button, const int expectedState, const std::function<void()>& action) : button(button),
		                                                                                               expectedState(expectedState),
		                                                                                               action(action) {
		}

		MouseBind(const int button, const std::function<void()>& action) : MouseBind(button, GLFW_PRESS, action) {}

		[[nodiscard]] int code() const {
			return button;
		}

		[[nodiscard]] int state() const {
			return expectedState;
		}

		[[nodiscard]] int getMod() const {
			return mod;
		}

		bool activated(GLFWwindow* window) const {
			return glfwGetMouseButton(window, button) == expectedState;
		}

		[[nodiscard]] bool activated(const int state) const {
			return expectedState == state;
		}

		void tryRun(const int state) const {
			if (activated(state))action();
		}

		void act() const {
			action();
		}
	};
}


