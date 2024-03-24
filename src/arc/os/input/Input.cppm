module;

#include <GLFW/glfw3.h>

export module Core.Input;

import std;

import Geom.Vector2D;
import Concepts;
import Ctrl.Constants;
import OS.KeyBind;
import OS.MouseBind;
import OS.InputListener;
import OS.ApplicationListener;

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
			requires requires(T t){
				t.state();
				t.code();
				t.tryRun(0, 0);
				t.act();
			}
		struct InputGroup final : OS::InputListener{
			array<vector<unique_ptr<T>>, total> binds{};
			array<vector<unique_ptr<T>>, total> continuous{};
			array<float, total> doubleClick{};

			static constexpr unsigned int Continuous = 0b1000'0000;
			array<unsigned char, total> pressed{};

			void insert(const int code, const int mode) {
				pressed[code] = static_cast<unsigned char>(mode | Continuous); //Just make it != 0
			}

			void remove(const int code) {
				pressed[code] = 0;
			}

			bool hasAction(const int key) const {
				return static_cast<bool>(pressed[key]);
			}

			void inform(const int code, int action, const int mods) override {
				switch(action) { // NOLINT(*-multiway-paths-covered)
					case Ctrl::Act_Press : {
						insert(code, mods);
						if(doubleClick[code] <= 0) {
							doubleClick[code] = Ctrl::doublePressMaxSpaceing;
						}else{
							doubleClick[code] = -1;
							action = Ctrl::Act_DoubleClick;
						}
						break;
					}
					case Ctrl::Act_Release : {
						remove(code);
						break;
					}
					default : break;
				}

				const auto& targets = binds[code];

				if (targets.empty())return;

				for (const auto& bind : targets) {
					bind->tryRun(action, mods);
				}
			}

			void update(const float delta) {
				for(int key = 0; key < pressed.size(); ++key) {
					if(pressed[key]) {
						for (const auto& bind : continuous[key]) {
							bind->tryRun(Ctrl::Act_Continuous, pressed[key]);
						}
					}
				}

				for(float& reload : doubleClick) {
					if(reload > 0)reload -= delta;
				}
			}

			void registerBind(T* bind) {
				auto& container = Ctrl::isContinuous(bind->state()) ? continuous : binds;

				container[bind->code()].emplace_back(std::make_unique<T>(*bind));
			}

			void clear() {
				std::ranges::for_each(binds, [](vector<unique_ptr<T>>& ptrs) {
					ptrs.clear();
				});

				std::ranges::for_each(continuous, [](vector<unique_ptr<T>>& ptrs) {
					ptrs.clear();
				});
			}
		};

		InputGroup<OS::MouseBind, Ctrl::MOUSE_BUTTON_COUNT> mouseGroup{};
		InputGroup<OS::KeyBind  , Ctrl::KEY_COUNT> keyGroup{};

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
		void registerMouseBind(const int button, const int expectedState, const int expectedMode, Func&& func) {
			registerMouseBind(new MouseBind{button, expectedState, expectedMode, std::forward<Func>(func)});
		}

		template<Concepts::Invokable<void()> Func>
		void registerKeyBind(const int key, const int expectedState, const int expectedMode, Func&& func) {
			registerKeyBind(new KeyBind{key, expectedState, expectedMode, std::forward<Func>(func)});
		}

		template<Concepts::Invokable<void()> Func>
		void registerMouseBind(const int button, const int expectedState, Func&& func) {
			registerMouseBind(new MouseBind{button, expectedState, std::forward<Func>(func)});
		}

		template<Concepts::Invokable<void()> Func>
		void registerKeyBind(const int key, const int expectedState, Func&& func) {
			registerKeyBind(new KeyBind{key, expectedState, std::forward<Func>(func)});
		}

		[[nodiscard]] bool isPressedKey(const int key) const{
			return keyGroup.hasAction(key);
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

			std::ranges::for_each(inputMouseListeners, [button, action, mods](InputListener* listener) {
				listener->inform(button, action, mods);
			});
		}

		void informKeyAction(const GLFWwindow* targetWin, const int key, [[maybe_unused]] int scanCode, const int action, [[maybe_unused]] const int mods) {
			keyGroup.inform(key, action, mods);

			std::ranges::for_each(inputKeyListeners, [key, action, mods](InputListener* listener) {
				listener->inform(key, action, mods);
			});
		}

		void cursorMoveInform(const float x, const float y) {
			mousePos.set(x, y);

			std::ranges::for_each(cursorMoveListeners, [x, y](const PosListener& listener) {
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

			std::ranges::for_each(scrollListeners, [x, y](const PosListener& listener) {
				listener(x, y);
			});
		}

		[[nodiscard]] bool cursorInbound() const {
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

			std::ranges::for_each(velocityListeners, [this](const PosListener& listener) {
				listener(mouseVelocity.x, mouseVelocity.y);
			});
		}

	};
}
