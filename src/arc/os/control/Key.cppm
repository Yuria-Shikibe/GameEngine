module;

#include <GLFW/glfw3.h>

export module OS.Ctrl.Bind:Key;
import :InputBind;

import std;

export namespace OS{
	struct KeyBind : InputBind{
		[[nodiscard]] KeyBind() = default;

		[[nodiscard]] KeyBind(const int key, const int expectedState, const int expectedMode, const bool ignoreMode,
		                      const std::function<void()>& action)
			: InputBind{key, expectedState, expectedMode, ignoreMode, action}{}

		[[nodiscard]] KeyBind(const int button, const int expected_state, const int expected_mode, const std::function<void()>& action)
			: InputBind{button, expected_state, expected_mode, action}{}

		[[nodiscard]] KeyBind(const int button, const int expected_state, const std::function<void()>& action)
			: InputBind{button, expected_state, action}{}

		[[nodiscard]] KeyBind(const int button, const std::function<void()>& action)
			: InputBind{button, action}{}

		bool activated(GLFWwindow* window) const {
			return glfwGetKey(window, key) == expectedAct;
		}
	};
}
