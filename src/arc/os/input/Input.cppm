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
	class Input final : public ApplicationListener {
	public:
		using PosListener = std::function<void(float, float)>;
		std::vector<PosListener> scrollListeners{};
		std::vector<PosListener> cursorMoveListeners{};
		std::vector<PosListener> velocityListeners{};

		std::vector<InputListener*> inputKeyListeners{};
		std::vector<InputListener*> inputMouseListeners{};

		template <typename T, size_t total>
			requires requires(T t){
				t.state();
				t.code();
				t.tryRun(0, 0);
				t.act();
			}
		struct InputGroup{
			std::array<std::vector<std::unique_ptr<T>>, total> binds{};
			std::array<std::vector<std::unique_ptr<T>>, total> continuous{};

			std::array<float, total> doubleClick{};

			/**
			 * @code
			 * | 0b'0000'0000'0000'0000
			 * |         CRrp|-- MOD --
			 * | C - Continuous
			 * | R - Repeat
			 * | r - release
			 * | p - press
			 * @endcode
			 */
			static constexpr unsigned short ModeMask = 0xff;
			static constexpr unsigned short DoubleClick = 0b0001'0000'0000'0000;
			static constexpr unsigned short Continuous = 0b1000'0000'0000;
			static constexpr unsigned short Repeat = 0b0100'0000'0000;
			static constexpr unsigned short Release = 0b0010'0000'0000;
			static constexpr unsigned short Press = 0b0001'0000'0000;
			static constexpr unsigned short RP_Eraser = ~(Release | Press | DoubleClick);
			std::array<unsigned short, total> pressed{};

			bool get(const int code, const int action, int mode){
				unsigned short actionTgt{};
				unsigned short target = pressed[code];

				if(mode != Ctrl::Mode_IGNORE){
					if((target & ModeMask) != (mode & ModeMask))return false;
				}

				switch(action){
					case Ctrl::Act_Press : actionTgt = Press;
						break;
					case Ctrl::Act_Continuous : actionTgt = Continuous;
						break;
					case Ctrl::Act_Release : actionTgt = Release;
						break;
					case Ctrl::Act_Repeat : actionTgt = Repeat;
						break;
					case Ctrl::Act_DoubleClick : actionTgt = DoubleClick;
						break;
					default : break;
				}

				return target & actionTgt;
			}

			bool insert(const int code, const int action, const int mode) {
				unsigned short pushIn = pressed[code] & ~ModeMask;

				bool isDoubleClick = false;

				switch(action){
					case Ctrl::Act_Press :{
						if(doubleClick[code] <= 0) {
							doubleClick[code] = Ctrl::doublePressMaxSpaceing;
							pushIn = Press | Continuous;
						}else{
							doubleClick[code] = -1;
							isDoubleClick = true;
							pushIn = Press | DoubleClick;
						}

						break;
					}
					case Ctrl::Act_Release : pushIn = Release; break;
					case Ctrl::Act_Repeat : pushIn = Repeat | Continuous; break;
					default: break;
				}

				pressed[code] = pushIn | (mode & ModeMask);

				return isDoubleClick;
			}

			// void remove(const int code) {
			// 	pressed[code] = 0;
			// }

			[[nodiscard]] bool hasAction(const int key) const {
				return static_cast<bool>(pressed[key]);
			}

			void inform(const int code, const int action, const int mods){
				bool doubleClick = insert(code, action, mods);

				const auto& targets = binds[code];

				if (!targets.empty()){
					for (const auto& bind : targets) {
						bind->tryRun(action, mods);
						if(doubleClick){
							bind->tryRun(Ctrl::Act_DoubleClick, mods);
						}
					}
				}
			}

			void update(const float delta) {
				for(int key = 0; key < pressed.size(); ++key) {
					pressed[key] &= RP_Eraser;
					if(pressed[key] & Continuous) {
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
				std::ranges::for_each(binds, [](std::vector<std::unique_ptr<T>>& ptrs) {
					ptrs.clear();
				});

				std::ranges::for_each(continuous, [](std::vector<std::unique_ptr<T>>& ptrs) {
					ptrs.clear();
				});
			}
		};

		InputGroup<OS::MouseBind, Ctrl::MOUSE_BUTTON_COUNT> mouseGroup{};
		InputGroup<OS::KeyBind  , Ctrl::KEY_COUNT> keyGroup{};

		//I think multi key bins are already enough for normal usage. Key + Mouse needn't be considered.
		std::vector<std::unique_ptr<KeyBindMultipleTarget>> multipleKeyBinds{};

	protected:
		bool activated{true};
		bool isInbound{false};

		Geom::Vec2 mousePos{};
		Geom::Vec2 lastMousePos{};
		Geom::Vec2 mouseVelocity{};
		Geom::Vec2 scrollOffset{};

		std::vector<Input*> subInputs{};

	public:
		explicit Input() = default;

		~Input() override = default;

		Input(const Input& other) = delete;

		Input(Input&& other) = delete;

		Input& operator=(const Input& other) = delete;

		Input& operator=(Input&& other) = delete;



		void registerMouseBind(MouseBind* mouseBind) {
			mouseGroup.registerBind(mouseBind);
		}

		void registerKeyBind(KeyBind* keyBind) {
			keyGroup.registerBind(keyBind);
		}

		template<Concepts::Invokable<void()> Func>
		void registerMouseBind(const int button, const int expectedState, const int expectedMode, Func&& func) {
			this->registerMouseBind(new MouseBind{button, expectedState, expectedMode, std::forward<Func>(func)});
		}

		template<Concepts::Invokable<void()> Func>
		void registerKeyBind(const int key, const int expectedState, const int expectedMode, Func&& func) {
			this->registerKeyBind(new KeyBind{key, expectedState, expectedMode, std::forward<Func>(func)});
		}

		template<Concepts::Invokable<void()> Func>
		void registerMouseBind(const int button, const int expectedState, Func&& func) {
			this->registerMouseBind(new MouseBind{button, expectedState, std::forward<Func>(func)});
		}

		template<Concepts::Invokable<void()> Func>
		void registerKeyBind(const int key, const int expectedState, Func&& func) {
			this->registerKeyBind(new KeyBind{key, expectedState, std::forward<Func>(func)});
		}

		template<Concepts::Invokable<void()> Func>
		void registerKeyBind(std::initializer_list<std::pair<int, int>> pairs, Func&& func) {

			for (auto [key, expectedState] : pairs){
				this->registerKeyBind(new KeyBind{key, expectedState, std::forward<Func>(func)});
			}
		}

		template<Concepts::Invokable<void()> Func>
		void registerKeyBind(std::initializer_list<std::tuple<int, int, int>> params, Func&& func) {
			for (auto [key, expectedState, expectedMode] : params){
				this->registerKeyBind(new KeyBind{key, expectedState, expectedMode, std::forward<Func>(func)});
			}
		}

		[[nodiscard]] bool isPressedKey(const int key) const{
			return keyGroup.hasAction(key);
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

		void registerSubInput(Input* input){
			subInputs.push_back(input);
		}

		void eraseSubInput(Input* input){
			std::erase(subInputs, input);
		}

		void activate(){activated = true;}

		void deactivate(){activated = false;}

		[[nodiscard]] bool isActivated() const {return activated;}

		/**
		 * \brief No remove function provided. If the input binds needed to be changed, clear it all and register the new binds by the action table.
		 * this is a infrequent operation so just keep the code clean.
		 */
		void informMouseAction(const int button, const int action, const int mods) {
			mouseGroup.inform(button, action, mods);

			for (const auto & listener : inputMouseListeners){
				listener->inform(button, action, mods);
			}

			for(auto* subInput : subInputs){
				if(subInput->activated)subInput->informMouseAction(button, action, mods);
			}
		}

		void informKeyAction(const int key, const int scanCode, const int action, const int mods) {
			keyGroup.inform(key, action, mods);

			for (const auto & listener : inputKeyListeners){
				listener->inform(key, action, mods);
			}

			for(auto* subInput : subInputs){
				if(subInput->activated)subInput->informKeyAction(key, scanCode, action, mods);
			}
		}

		void cursorMoveInform(const float x, const float y) {
			mousePos.set(x, y);

			for (auto& listener : cursorMoveListeners){
				listener(x, y);
			}

			for(auto* subInput : subInputs){
				if(subInput->activated)subInput->cursorMoveInform(x, y);
			}
		}

		[[nodiscard]] Geom::Vec2 getMousePos() const {
			return mousePos;
		}

		[[nodiscard]] Geom::Vec2 getScrollOffset() const {
			return scrollOffset;
		}

		void setScrollOffset(const float x, const float y) {
			scrollOffset.set(-x, y);

			for (auto& listener : scrollListeners){
				listener(-x, y);
			}

			for(auto* subInput : subInputs){
				if(subInput->activated)subInput->setScrollOffset(x, y);
			}
		}

		[[nodiscard]] bool cursorInbound() const {
			return isInbound;
		}

		void setInbound(const bool b) {
			isInbound = b;

			for(auto* subInput : subInputs){
				if(subInput->activated)subInput->setInbound(b);
			}
		}

		void updatePost(const float delta) override {
			for (const auto& multipleKeyBind : multipleKeyBinds) {
				multipleKeyBind->resetState(delta);
			}

			keyGroup.update(delta);
			mouseGroup.update(delta);

			mouseVelocity = mousePos;
			mouseVelocity -= lastMousePos;
			mouseVelocity /= delta;

			lastMousePos = mousePos;

			for (auto& listener : velocityListeners){
				listener(mouseVelocity.x, mouseVelocity.y);
			}

			for(auto* subInput : subInputs){
				if(subInput->activated)subInput->updatePost(delta);
			}
		}

	};
}
