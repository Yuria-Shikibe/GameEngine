module ;

export module OS.KeyBind;

import <GLFW/glfw3.h>;
import <functional>;
import <ranges>;

export namespace OS{
	struct KeyBind {
	protected:
		int key = GLFW_KEY_UNKNOWN;
		int expectedState = GLFW_PRESS;
		int expectedMode = 0;
		bool ignoreMode{true};

		std::function<void()> action = nullptr;

	public:
		[[nodiscard]] KeyBind(const int key, const int expected_state, const int expected_mode,
			const std::function<void()>& action)
			: key(key),
			expectedState(expected_state),
			expectedMode(expected_mode),
			action(action), ignoreMode(false) {
		}

		[[nodiscard]] KeyBind(const int key, const int expected_state,
			const std::function<void()>& action)
			: key(key),
			expectedState(expected_state),
			action(action) {
		}

		KeyBind(const int key, const int expectedState) : KeyBind(key, expectedState, nullptr) {
		}

		explicit KeyBind(const std::function<void()>& action) : action(action) {
		}

		virtual ~KeyBind() = default;

		KeyBind(const KeyBind& other) = default;

		KeyBind(KeyBind&& other) noexcept : key(other.key),
		                                    expectedState(other.expectedState),
		                                    action(std::move(other.action)) {
		}

		KeyBind& operator=(const KeyBind& other) {
			if (this == &other) return *this;
			action = other.action;
			key = other.key;
			expectedState = other.expectedState;
			return *this;
		}

		KeyBind& operator=(KeyBind&& other) noexcept {
			if (this == &other) return *this;
			action = std::move(other.action);
			key = other.key;
			expectedState = other.expectedState;
			return *this;
		}

		bool activated(GLFWwindow* window) const {
			return glfwGetKey(window, key) == expectedState;
		}

		[[nodiscard]] bool activated(const int state, const int mode) const {
			return state == expectedState && modeMatch(mode);
		}

		[[nodiscard]] int code() const {
			return key;
		}

		[[nodiscard]] int state() const {
			return expectedState;
		}

		void tryRun(const int state, const int mode) const {
			if (activated(state, mode))act();
		}

		[[nodiscard]] bool modeMatch(const int mode) const {
			return ignoreMode || (mode & 0xff) == expectedMode;
		}

		void act() const {
			action();
		}

		friend bool operator==(const KeyBind& lhs, const KeyBind& rhs) {
			return lhs.key == rhs.key && lhs.expectedState == rhs.expectedState && &lhs.action == &rhs.action;
		}

		friend bool operator!=(const KeyBind& lhs, const KeyBind& rhs) {
			return !(lhs == rhs);
		}
	};

	struct KeyBindMultipleTarget {
	protected:
		std::unordered_map<int, bool> linkedMap{};

		float resetFrameDelta = 0;
		static constexpr float expectedResetFrameDelta = 30.0f; //TODO this should vary from user's display refresh rate.

		int count = 0;
		int expectedCount = 0;

		std::function<void()> action = nullptr;

	public:
		//Should Be Called!
		void registerRequired(const std::span<KeyBind>& arr) {
			for (const auto& keyBind : arr) {
				linkedMap.insert_or_assign(keyBind.code(), false);
			}

			expectedCount = static_cast<int>(arr.size());
		}

		void trigger(const KeyBind& bind) {
			if (bool& keyState = linkedMap.at(bind.code()); !keyState) {
				keyState = true;
				count++;
			}

			if (count == expectedCount) {
				action();
				resetState(expectedResetFrameDelta);
			}
		}

   		void resetState(const float delta) {
			if(count > 0)resetFrameDelta += delta;

			if(resetFrameDelta >= expectedResetFrameDelta) {
				for (bool& val : linkedMap | std::views::values) {
					val = false;
				}

				count = 0;

				resetFrameDelta = 0;
			}
		}

		explicit KeyBindMultipleTarget(const std::function<void()>& action) : action(action) {
		}

		explicit KeyBindMultipleTarget(std::function<void()>&& action) : action(std::move(action)) {
		}

		KeyBindMultipleTarget(const std::function<void()>& action, const std::span<KeyBind>& arr) : action(action) {
			registerRequired(arr);
		}
	};

	struct KeyBindMultiple final : KeyBind {
	protected:
		KeyBindMultipleTarget* linkedTarget = nullptr;

	public:
		KeyBindMultiple(const int key_, const int expectedState_, KeyBindMultipleTarget* linkedTarget) : KeyBind(key_, expectedState_, [this]() {this->linkedTarget->trigger(*this);}),
		                                                                                                 linkedTarget(linkedTarget) {
		}

		KeyBindMultiple(const int key_, KeyBindMultipleTarget* linkedTarget) : KeyBindMultiple(key_, GLFW_PRESS, linkedTarget) {
		}


		KeyBindMultiple(const KeyBind& keyBind, KeyBindMultipleTarget* linkedTarget) : KeyBindMultiple(keyBind.code(), keyBind.state(), linkedTarget) {

		}
	};
}
