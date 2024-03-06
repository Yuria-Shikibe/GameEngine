module;

export module OS.MouseBind;

import <GLFW/glfw3.h>;
import <functional>;

export namespace OS{
	struct MouseBind {
	protected:
		int button = 0;
		int expectedState = 1;
		int expectedMode = 0;
		bool ignoreMode{true};

		std::function<void()> action = nullptr;

	public:
		[[nodiscard]] MouseBind(const int button, const int expected_state, const int expected_mode,
			const std::function<void()>& action)
			: button(button),
			expectedState(expected_state),
			expectedMode(expected_mode),
			action(action), ignoreMode(false) {
		}

		[[nodiscard]] MouseBind(const int button, const int expected_state,
			const std::function<void()>& action)
			: button(button),
			expectedState(expected_state),
			action(action) {
		}

		MouseBind(const int button, const std::function<void()>& action) : MouseBind(button, GLFW_PRESS, action) {}

		[[nodiscard]] int code() const {
			return button;
		}

		[[nodiscard]] int state() const {
			return expectedState;
		}

		bool activated(GLFWwindow* window) const {
			return glfwGetMouseButton(window, button) == expectedState;
		}

		[[nodiscard]] bool activated(const int state, const int mode) const {
			return expectedState == state && modeMatch(mode);
		}

		[[nodiscard]] bool modeMatch(const int mode) const {
			return ignoreMode || (mode & 0xff) == expectedMode;
		}

		void tryRun(const int state, const int mode) const {
			if (activated(state, mode))action();
		}

		void act() const {
			action();
		}
	};
}


