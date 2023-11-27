//
// Created by Matrix on 2023/11/19.
//

module;

export module Core.Input;

import <array>;
import <functional>;
import <set>;
import <memory>;

import <GLFW/glfw3.h>;

import Geom.Vector2D;
import Concepts;
import OS.Key;
import OS.Mouse;
import OS.ApplicationListener;
import <span>;

using namespace OS;

export namespace Core{

	using std::vector;
	using std::set;
	using std::function;
	using std::array;
	using std::unique_ptr;

	class Input final : public virtual ApplicationListener {
	public:
		vector<function<void(float, float)>> scrollListener{};
		vector<function<void(float, float)>> cursorListener{};

		//Should Increase the abstract level? Or just...
		array<vector<unique_ptr<KeyBind>>, GLFW_KEY_LAST> keys{};
		array<vector<unique_ptr<KeyBind>>, GLFW_KEY_LAST> keys_frequentCheck{};
		set<int> pressedKeys{};


		//TODO Seriously, this array is pretty small, and most calls are LMB or RMB, may be directly using a array instead of a set is faster?
		array<vector<unique_ptr<MouseBind>>, GLFW_MOUSE_BUTTON_8> mouseBinds{};
		array<vector<unique_ptr<MouseBind>>, GLFW_MOUSE_BUTTON_8> mouseBinds_frequentCheck{};
		array<bool, GLFW_MOUSE_BUTTON_8 + 1> pressedMouseButtons{};

		//I think multi key bins are already enough for normal usage. Key + Mouse needn't be considered.
		vector<unique_ptr<KeyBindMultipleTarget>> multipleKeyBinds{};

	protected:
		GLFWwindow* window{nullptr};
		bool isInbound{false};
		Geom::Vector2D mousePos{};
		Geom::Vector2D scrollOffset{};

	public:
		explicit Input(GLFWwindow* w) : window(w) {
		}

		~Input() override = default;

		Input(const Input& other) = delete;

		Input(Input&& other) = delete;

		Input& operator=(const Input& other) = delete;

		Input& operator=(Input&& other) = delete;

		void registerMouseBind(const bool frequentCheck, MouseBind* mouseBind) {
			array<vector<unique_ptr<MouseBind>>, GLFW_MOUSE_BUTTON_8>& container = frequentCheck ? mouseBinds_frequentCheck : mouseBinds;

			container[mouseBind->getButton()].push_back(std::make_unique<MouseBind>(*mouseBind));
		}

		/**
		 * \brief No remove function provided. If the input binds needed to be changed, clear it all and register the new binds by the action table.
		 * this is a infrequent operation so just keep the code clean.
		 */

		void informMouseAction(const GLFWwindow* targetWin, const int button, const int action, [[maybe_unused]] int mods) {
			if (targetWin != window)return;

			switch(action) {
				case GLFW_PRESS : pressedMouseButtons[button] = true; break;
				case GLFW_RELEASE : pressedMouseButtons[button] = false; break;
				default : break;
			}

			const vector<unique_ptr<MouseBind>>& binds = mouseBinds[button];

			if (binds.empty())return;

			for (const auto& bind : binds) {
				bind->tryRun(action);
			}
		}

		template<Concepts::Invokable<void(int)> Func>
		void registerKeyBind(const bool frequentCheck, const int key, const int expectedState, Func&& func) {
			registerKeyBind(frequentCheck, new KeyBind{key, expectedState, func});
		}

		void registerKeyBind(const bool frequentCheck, KeyBind* keyBind) {
			array<vector<unique_ptr<KeyBind>>, GLFW_KEY_LAST>& container = frequentCheck ? keys_frequentCheck : keys;

			container[keyBind->keyCode()].push_back(std::make_unique<KeyBind>(*keyBind));
		}

		void registerKeyBindMulti(const bool frequentCheck, const std::span<KeyBind>& keyBinds,
		                          std::function<void()>&& act) {
			const auto target = new KeyBindMultipleTarget{ act, keyBinds };

			for (const auto& keyBind : keyBinds) {
				registerKeyBind(frequentCheck, new KeyBindMultiple{ keyBind, target });
			}

			multipleKeyBinds.emplace_back(target);
		}

		void clearAllBinds() {
			for (auto& keyBinds : keys) {
				keyBinds.clear();
			}

			for (auto& keyBinds : keys_frequentCheck) {
				keyBinds.clear();
			}

			for (auto& keyBinds : mouseBinds) {
				keyBinds.clear();
			}

			for (auto& keyBinds : mouseBinds_frequentCheck) {
				keyBinds.clear();
			}

			multipleKeyBinds.clear();
		}

		void informKeyAction(const GLFWwindow* targetWin, const int key, [[maybe_unused]] int scanCode, const int action, [[maybe_unused]] int mods) {
			if (targetWin != window)return;

			if (action == GLFW_PRESS) {
				pressedKeys.insert(key);
			}
			else if (action == GLFW_RELEASE) {
				pressedKeys.erase(key);
			}

			const vector<unique_ptr<KeyBind>>& binds = keys[key];

			if (binds.empty())return;

			for (const auto& bind : binds) {
				bind->tryRun(action);
			}
		}

		void setPos(const float x, const float y) {
			mousePos.set(x, y);

			for (const auto& listener : cursorListener) {
				listener(x, y);
			}
		}

		Geom::Vector2D& getPos() {
			return mousePos;
		}

		Geom::Vector2D& getScrollOffset() {
			return scrollOffset;
		}

		void setScrollOffset(const float x, const float y) {
			scrollOffset.set(x, y);

			for (const auto& listener : scrollListener) {
				listener(x, y);
			}
		}

		[[nodiscard]] bool inbound() const {
			return isInbound;
		}

		void setInbound(const bool b) {
			isInbound = b;
		}

		void update() override {
			for (const auto& multipleKeyBind : multipleKeyBinds) {
				multipleKeyBind->resetState();
			}

			for (const int key : pressedKeys) {
				for (const auto& keyBind : keys_frequentCheck[key]) {
					if (keyBind->activated(window))keyBind->act();
				}
			}

			for(int key = 0; key < pressedMouseButtons.size(); ++key) {
				if(pressedMouseButtons[key]) {
					for (const auto& mouseBind : mouseBinds_frequentCheck[key]) {
						if (mouseBind->activated(window))mouseBind->act();
					}
				}
			}
		}

	};
}
