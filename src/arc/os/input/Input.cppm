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
import Ctrl.Constants;
import OS.Key;
import OS.Mouse;
import OS.InputListener;
import OS.ApplicationListener;
import <algorithm>;
import <execution>;
import <span>;
import <unordered_set>;

using namespace OS;

export namespace Core{
	using std::vector;
	using std::set;
	using std::pair;
	using std::function;
	using std::array;
	using std::unique_ptr;

	class Input final : public ApplicationListener {
	public:
		using PosListener = function<void(float, float)>;
		vector<PosListener> scrollListeners{};
		vector<PosListener> cursorMoveListeners{};
		vector<PosListener> velocityListeners{};

		vector<InputListener*> inputKeyListeners{};
		vector<InputListener*> inputMouseListeners{};


		template <typename T, size_t total>
		struct ArrayChecker final{
			array<bool, total> pressed{};

			void insert(const int code) {
				pressed[code] = true;
			}

			void remove(const int code) {
				pressed[code] = false;
			}

			void operator()(const array<vector<unique_ptr<T>>, total>& range){
				for(int key = 0; key < range.size(); ++key) {
					if(pressed[key]) {
						for (const auto& mouseBind : range[key]) {
							mouseBind->act();
						}
					}
				}
			}
		};

		template <typename T, size_t total>
		struct SetChecker final{
			std::unordered_set<int> pressed{};

			void insert(const int code) {
				pressed.insert(code);
			}

			void remove(const int code) {
				pressed.erase(code);
			}

			void operator()(const array<vector<unique_ptr<T>>, total>& range){
				for(const int key : pressed) {
					for (const auto& mouseBind : range[key]) {
						mouseBind->act();
					}
				}
			}
		};

		template <typename T, size_t total, typename Checker>
			requires requires(T t){t.state();t.code();t.tryRun(0);t.act();} && Concepts::DefConstructable<Checker>
		struct InputGroup final : OS::InputListener{
			array<vector<unique_ptr<T>>, total> binds{};
			array<vector<unique_ptr<T>>, total> continuous{};
			array<float, total> doubleClick{};
			Checker pressed{};

			void inform(const int code, int action, const int mods) override {
				switch(action) { // NOLINT(*-multiway-paths-covered)
					case Ctrl::Act_Press : {
						pressed.insert(code);
						if(doubleClick[code] <= 0) {
							doubleClick[code] = Ctrl::doublePressMaxSpaceing;
						}else{
							doubleClick[code] = -1;
							action = Ctrl::Act_DoubleClick;
						}
						break;
					}
					case Ctrl::Act_Release : {
						pressed.remove(code);
						break;
					}
					default : break;
				}

				const auto& targets = binds[code];

				if (targets.empty())return;

				for (const auto& bind : targets) {
					bind->tryRun(action);
				}
			}

			void update(const float delta) {
				pressed(continuous);

				for(float& reload : doubleClick) {
					if(reload > 0)reload -= delta;
				}
			}

			void registerBind(T* bind) {
				auto& container = Ctrl::isContinuous(bind->state()) ? continuous : binds;

				if(Ctrl::isContinuous(bind->state())) {

				}

				container[bind->code()].emplace_back(std::make_unique<T>(*bind));
			}

			void clear() {
				std::for_each(std::execution::par, binds.begin(), binds.end(), [](vector<unique_ptr<T>>& ptrs) {
					ptrs.clear();
				});

				std::for_each(std::execution::par, continuous.begin(), continuous.end(), [](vector<unique_ptr<T>>& ptrs) {
					ptrs.clear();
				});
			}
		};

		InputGroup<MouseBind, Ctrl::MOUSE_BUTTON_COUNT, ArrayChecker<MouseBind, Ctrl::MOUSE_BUTTON_COUNT>> mouseGroup{};
		InputGroup<KeyBind  , Ctrl::KEY_COUNT	     , SetChecker<KeyBind, Ctrl::KEY_COUNT>> keyGroup{};

		//I think multi key bins are already enough for normal usage. Key + Mouse needn't be considered.
		vector<unique_ptr<KeyBindMultipleTarget>> multipleKeyBinds{};

	protected:
		GLFWwindow* window{nullptr};
		bool isInbound{false};

		Geom::Vec2 mousePos{};
		Geom::Vec2 lastMousePos{};

		Geom::Vec2 mouseVelocity{};

		Geom::Vec2 scrollOffset{};

	public:
		explicit Input(GLFWwindow* w) : window(w) {
		}

		~Input() override = default;

		Input(const Input& other) = delete;

		Input(Input&& other) = delete;

		Input& operator=(const Input& other) = delete;

		Input& operator=(Input&& other) = delete;

		void registerMouseBind(MouseBind* mouseBind) {
			mouseGroup.registerBind(mouseBind);
		}

		template<Concepts::Invokable<void()> Func>
		void registerMouseBind(const int button, const int expectedState, Func&& func) {
			registerMouseBind(new MouseBind{button, expectedState, func});
		}

		template<Concepts::Invokable<void()> Func>
		void registerKeyBind(const int key, const int expectedState, Func&& func) {
			registerKeyBind(new KeyBind{key, expectedState, func});
		}

		void registerKeyBind(KeyBind* keyBind) {

			keyGroup.registerBind(keyBind);
		}

		void registerKeyBindMulti(const std::span<KeyBind>& keyBinds,
		                          std::function<void()>&& act) {
			const auto target = new KeyBindMultipleTarget{ act, keyBinds };

			for (const auto& keyBind : keyBinds) {
				registerKeyBind(new KeyBindMultiple{ keyBind, target });
			}

			multipleKeyBinds.emplace_back(target);
		}

		void clearAllBinds() {
			keyGroup.clear();
			mouseGroup.clear();
			multipleKeyBinds.clear();
		}

		/**
		 * \brief No remove function provided. If the input binds needed to be changed, clear it all and register the new binds by the action table.
		 * this is a infrequent operation so just keep the code clean.
		 */
		void informMouseAction(const GLFWwindow* targetWin, const int button, const int action, [[maybe_unused]] const int mods) {
			mouseGroup.inform(button, action, mods);

			std::for_each(std::execution::par, inputMouseListeners.begin(), inputMouseListeners.end(), [button, action, mods](InputListener* listener) {
				listener->inform(button, action, mods);
			});
		}

		void informKeyAction(const GLFWwindow* targetWin, const int key, [[maybe_unused]] int scanCode, const int action, [[maybe_unused]] const int mods) {
			keyGroup.inform(key, action, mods);

			std::for_each(std::execution::par, inputKeyListeners.begin(), inputKeyListeners.end(), [key, action, mods](InputListener* listener) {
				listener->inform(key, action, mods);
			});
		}

		void cursorMoveInform(const float x, const float y) {
			mousePos.set(x, y);

			std::for_each(std::execution::par, cursorMoveListeners.begin(), cursorMoveListeners.end(), [x, y](const PosListener& listener) {
				listener(x, y);
			});
		}

		[[nodiscard]] Geom::Vec2 getMousePos() const {
			return mousePos;
		}

		[[nodiscard]] Geom::Vec2 getScrollOffset() const {
			return scrollOffset;
		}

		void setScrollOffset(const float x, const float y) {
			scrollOffset.set(x, y);

			std::for_each(std::execution::par, scrollListeners.begin(), scrollListeners.end(), [x, y](const PosListener& listener) {
				listener(x, y);
			});
		}

		[[nodiscard]] bool inbound() const {
			return isInbound;
		}

		void setInbound(const bool b) {
			isInbound = b;
		}

		void update(const float delta) override {
			for (const auto& multipleKeyBind : multipleKeyBinds) {
				multipleKeyBind->resetState(delta);
			}

			keyGroup.update(delta);
			mouseGroup.update(delta);

			mouseVelocity = mousePos;
			mouseVelocity -= lastMousePos;
			mouseVelocity /= delta;

			lastMousePos = mousePos;

			std::for_each(std::execution::par, velocityListeners.begin(), velocityListeners.end(), [this](const PosListener& listener) {
				listener(mouseVelocity.x, mouseVelocity.y);
			});
		}

	};
}
