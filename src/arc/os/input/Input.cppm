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

	class Input final : public virtual ApplicationListener {
	public:
		unsigned int refreshRate = 60;
		vector<function<void(float, float)>> scrollListener{};
		vector<function<void(float, float)>> cursorListener{};

		template <typename T, size_t total = 0>
		struct PressedChecker {
			virtual ~PressedChecker() = default;

			virtual void insert(int code) = 0;
			virtual void remove(int code) = 0;
			virtual void operator()(const array<vector<unique_ptr<T>>, total>& range) = 0;
		};

		template <typename T, size_t total = 0>
		struct ArrayChecker final : PressedChecker<T, total>{
			array<bool, total> pressed{};

			void insert(const int code) override {
				pressed[code] = true;
			}

			void remove(const int code) override {
				pressed[code] = false;
			}

			void operator()(const array<vector<unique_ptr<T>>, total>& range) override {
				for(int key = 0; key < range.size(); ++key) {
					if(pressed[key]) {
						for (const auto& mouseBind : range[key]) {
							mouseBind->act();
						}
					}
				}
			}
		};

		template <typename T, size_t total = 0>
		struct SetChecker final : PressedChecker<T, total>{
			std::unordered_set<int> pressed{};

			void insert(const int code) override {
				pressed.insert(code);
			}

			void remove(const int code) override {
				pressed.erase(code);
			}

			void operator()(const array<vector<unique_ptr<T>>, total>& range) override {
				for(const int key : pressed) {
					for (const auto& mouseBind : range[key]) {
						mouseBind->act();
					}
				}
			}
		};

		template <typename T, size_t total, Concepts::Derived<PressedChecker<T, total>> Checker>
			requires requires(T t){t.state();t.code();t.tryRun(0);t.act();} && Concepts::HasDefConstructor<Checker>
		struct InputGroup {
			array<vector<unique_ptr<T>>, total> binds{};
			array<vector<unique_ptr<T>>, total> continuous{};
			array<float, total> doubleClick{};
			Checker pressed{};

			void inform(const int code, int action, const int mods) {
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
				std::for_each(std::execution::par_unseq, binds.begin(), binds.end(), [](vector<unique_ptr<T>>& ptrs) {
					ptrs.clear();
				});

				std::for_each(std::execution::par_unseq, continuous.begin(), continuous.end(), [](vector<unique_ptr<T>>& ptrs) {
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
		}

		void informKeyAction(const GLFWwindow* targetWin, const int key, [[maybe_unused]] int scanCode, const int action, [[maybe_unused]] const int mods) {
			keyGroup.inform(key, action, mods);
		}

		void setPos(const float x, const float y) {
			mousePos.set(x, y);

			for (const auto& listener : cursorListener) {
				listener(x, y);
			}
		}

		Geom::Vector2D& getMousePos() {
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

		void update(const float delta) override {
			for (const auto& multipleKeyBind : multipleKeyBinds) {
				multipleKeyBind->resetState(delta);
			}

			keyGroup.update(delta);
			mouseGroup.update(delta);
		}

	};
}
