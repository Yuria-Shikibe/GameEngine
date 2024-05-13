module;

#include <GLFW/glfw3.h>

export module OS.Ctrl.Bind:Mouse;
import :InputBind;

import std;

export namespace OS{
	struct MouseBind : InputBind{
		[[nodiscard]] MouseBind() = default;

		[[nodiscard]] MouseBind(const int key, const int expectedState, const int expectedMode, const bool ignoreMode,
		                        const std::function<void()>& action)
			: InputBind{key, expectedState, expectedMode, ignoreMode, action}{}

		[[nodiscard]] MouseBind(const int button, const int expected_state, const int expected_mode, const std::function<void()>& action)
			: InputBind{button, expected_state, expected_mode, action}{}

		[[nodiscard]] MouseBind(const int button, const int expected_state, const std::function<void()>& action)
			: InputBind{button, expected_state, action}{}

		[[nodiscard]] MouseBind(const int button, const std::function<void()>& action)
			: InputBind{button, action}{}

		bool queryActivated(GLFWwindow* window) const noexcept{
			return glfwGetMouseButton(window, getKey()) == expectedAct;
		}
	};
}


