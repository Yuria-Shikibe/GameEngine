export module OS.Ctrl.Bind:InputBind;

import OS.Ctrl.Bind.Constants;
import std;

export namespace OS{
	struct InputBind{
	protected:
		int key = Ctrl::Unknown;
		int expectedAct = Ctrl::Act::Press;
		int expectedMode = 0;
		bool ignoreMode{true};

		std::function<void()> action = nullptr;

	public:
		[[nodiscard]] InputBind() = default;

		[[nodiscard]] InputBind(const int key, const int expectedState, const int expectedMode,
		                            const bool ignoreMode,
		                            const std::function<void()>& action)
			: key{key},
			  expectedAct{expectedState},
			  expectedMode{expectedMode},
			  ignoreMode{ignoreMode},
			  action{action}{}

		[[nodiscard]] InputBind(const int button, const int expected_state, const int expected_mode,
		                            const std::function<void()>& action)
			: InputBind(button, expected_state, expected_mode, expected_mode == Ctrl::Mode::Ignore, action){}

		[[nodiscard]] InputBind(const int button, const int expected_state, const std::function<void()>& action)
			: InputBind{button, expected_state, Ctrl::Mode::Ignore, action}{}

		[[nodiscard]] InputBind(const int button, const std::function<void()>& action) : InputBind(
			button, Ctrl::Act::Press, action){}


		[[nodiscard]] constexpr int getKey() const noexcept{
			return key;
		}

		[[nodiscard]] constexpr int getFullKey() const noexcept{
			return Ctrl::getFullKey(key, expectedAct, expectedMode);
		}

		[[nodiscard]] constexpr int state() const noexcept{
			return expectedAct;
		}

		[[nodiscard]] constexpr int mode() const noexcept{
			return expectedMode;
		}

		[[nodiscard]] constexpr bool activated(const int state, const int mode) const noexcept{
			return expectedAct == state && modeMatch(mode);
		}

		[[nodiscard]] constexpr bool modeMatch(const int mode) const noexcept{
			return ignoreMode || Ctrl::Mode::modeMatch(mode, expectedMode);
		}

		void tryRun(const int state, const int mode) const {
			if (activated(state, mode))act();
		}

		void act() const {
			action();
		}

		friend bool operator==(const InputBind& lhs, const InputBind& rhs) {
			return lhs.key == rhs.key && lhs.expectedAct == rhs.expectedAct && &lhs.action == &rhs.action;
		}

		friend bool operator!=(const InputBind& lhs, const InputBind& rhs) {
			return !(lhs == rhs);
		}
	};
}
