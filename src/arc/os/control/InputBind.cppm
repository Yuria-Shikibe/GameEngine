export module OS.Ctrl.Bind:InputBind;

import OS.Ctrl.Bind.Constants;
import std;
import ext.Concepts;

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

		[[nodiscard]] bool isIgnoreMode() const{ return ignoreMode; }

		void setIgnoreMode(const bool ignoreMode){ this->ignoreMode = ignoreMode; }

		void setKey(const int key){ this->key = key; }

		void setState(const int expectedAct){ this->expectedAct = expectedAct; }

		void setMode(const int expectedMode){ this->expectedMode = expectedMode; }

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

		friend constexpr bool operator==(const InputBind& lhs, const InputBind& rhs) {
			return lhs.key == rhs.key && lhs.expectedAct == rhs.expectedAct && &lhs.action == &rhs.action;
		}

		friend constexpr bool operator!=(const InputBind& lhs, const InputBind& rhs) {
			return !(lhs == rhs);
		}
	};

	class InputBindGroup{
		class InputGroup{
			using Signal = int;
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
			static constexpr Signal ModeMask = 0xff;
			static constexpr Signal DoubleClick = 0b0001'0000'0000'0000;
			static constexpr Signal Continuous = 0b1000'0000'0000;
			static constexpr Signal Repeat = 0b0100'0000'0000;
			static constexpr Signal Release = 0b0010'0000'0000;
			static constexpr Signal Press = 0b0001'0000'0000;
			static constexpr Signal RP_Eraser = ~(Release | Press | DoubleClick);

			std::array<std::vector<OS::InputBind>, Ctrl::AllKeyCount> binds{};
			std::array<std::vector<OS::InputBind>, Ctrl::AllKeyCount> continuous{};
			std::array<float, Ctrl::AllKeyCount> doubleClick{};
			std::array<unsigned short, Ctrl::AllKeyCount> pressed{};

			std::set<int> markedSignal{};

		private:
			void updateSignal(const int code){
				if(code == Ctrl::Act::Release){
					markedSignal.erase(code);
				}else{
					markedSignal.insert(code);
				}
			}

			/**
			 * @return isDoubleClick
			 */
			bool insert(const int code, const int action, const int mode){
				Signal pushIn = pressed[code] & ~ModeMask;

				bool isDoubleClick = false;

				switch(action){
					case Ctrl::Act::Press :{
						if(doubleClick[code] <= 0){
							doubleClick[code] = Ctrl::doublePressMaxSpaceing;
							pushIn = Press | Continuous;
						} else{
							doubleClick[code] = -1;
							isDoubleClick = true;
							pushIn = Press | DoubleClick;
						}

						break;
					}
					case Ctrl::Act::Release : pushIn = Release;
						break;
					case Ctrl::Act::Repeat : pushIn = Repeat | Continuous;
						break;
					default : break;
				}

				pressed[code] = pushIn | (mode & ModeMask);

				return isDoubleClick;
			}

		public:
			bool get(const int code, const int action, const int mode) const{
				Signal actionTgt{};
				const Signal target = pressed[code];

				if(mode != Ctrl::Mode::Ignore){
					if((target & ModeMask) != (mode & ModeMask)) return false;
				}

				switch(action){
					case Ctrl::Act::Press : actionTgt = Press;
					break;
					case Ctrl::Act::Continuous : actionTgt = Continuous;
					break;
					case Ctrl::Act::Release : actionTgt = Release;
					break;
					case Ctrl::Act::Repeat : actionTgt = Repeat;
					break;
					case Ctrl::Act::DoubleClick : actionTgt = DoubleClick;
					break;
					default : break;
				}

				return target & actionTgt;
			}

			[[nodiscard]] bool hasAction(const int code) const{
				return static_cast<bool>(pressed[code]);
			}

			void inform(const int code, const int action, const int mods){
				updateSignal(code);
				const bool doubleClick = insert(code, action, mods);

				const auto& targets = binds[code];

				if(!targets.empty()){
					for(const auto& bind : targets){
						bind.tryRun(action, mods);
						if(doubleClick){
							//TODO better double click event
							bind.tryRun(Ctrl::Act::DoubleClick, mods);
						}
					}
				}
			}

			void update(const float delta){
				for(const int key : markedSignal){
					pressed[key] &= RP_Eraser;
					if(pressed[key] & Continuous){
						for(const auto& bind : continuous[key]){
							bind.tryRun(Ctrl::Act::Continuous, pressed[key]);
						}
					}
				}

				for(float& reload : doubleClick){
					if(reload > 0) reload -= delta;
				}
			}

			void registerBind(OS::InputBind&& bind){
				if(bind.getKey() >= Ctrl::AllKeyCount)return;
				auto& container = Ctrl::isContinuous(bind.state()) ? continuous : binds;

				container[bind.getKey()].emplace_back(std::move(bind));
			}

			void clear(){
				std::ranges::for_each(binds, &std::vector<OS::InputBind>::clear);
				std::ranges::for_each(continuous, &std::vector<OS::InputBind>::clear);
			}
		};

		bool activated{true};

	public:
		InputGroup registerList{};

		void registerBind(OS::InputBind&& bind){
			registerList.registerBind(std::move(bind));
		}

		template <Concepts::Invokable<void()> Func>
		void registerBind(const int key, const int expectedState, const int expectedMode, Func&& func){
			registerBind(OS::InputBind{key, expectedState, expectedMode, std::forward<Func>(func)});
		}

		template <Concepts::Invokable<void()> Func>
		void registerBind(const int key, const int expectedState, Func&& func){
			registerBind(OS::InputBind{key, expectedState, std::forward<Func>(func)});
		}

		template <Concepts::Invokable<void()> Func1, Concepts::Invokable<void()> Func2>
		void registerBind(const int key, const int expectedMode, Func1&& onPress, Func2&& onRelease){
			registerBind(OS::InputBind{key, Ctrl::Act::Press, expectedMode, std::forward<Func1>(onPress)});
			registerBind(OS::InputBind{
					key, Ctrl::Act::Release, expectedMode, std::forward<Func2>(onRelease)
				});
		}

		template <Concepts::Invokable<void()> Func>
		void registerBind(std::initializer_list<std::pair<int, int>> pairs, Func&& func){
			for(auto [key, expectedState] : pairs){
				registerBind(OS::InputBind{key, expectedState, std::forward<Func>(func)});
			}
		}

		template <Concepts::Invokable<void()> Func>
		void registerBind(std::initializer_list<std::pair<int, int>> pairs, const int expectedMode, Func&& func){
			for(auto [key, expectedState] : pairs){
				registerBind(OS::InputBind{key, expectedState, expectedMode, std::forward<Func>(func)});
			}
		}

		template <Concepts::Invokable<void()> Func>
		void registerBind(std::initializer_list<std::tuple<int, int, int>> params, Func&& func){
			for(auto [key, expectedState, expectedMode] : params){
				registerBind(OS::InputBind{key, expectedState, expectedMode, std::forward<Func>(func)});
			}
		}

		[[nodiscard]] bool isPressedKey(const int key) const{
			return registerList.hasAction(key);
		}

		void clearAllBinds(){
			registerList = InputGroup{};
		}

		void update(const float delta){
			if(!activated) return;

			registerList.update(delta);
		}

		/**
		 * \brief No remove function provided. If the input binds needed to be changed, clear it all and register the new binds by the action table.
		 * this is a infrequent operation so just keep the code clean.
		 */
		void informMouseAction(const int key, const int action, const int mods){
			if(activated) registerList.inform(key, action, mods);
		}

		void informKeyAction(const int key, const int action, const int mods){
			if(activated) registerList.inform(key, action, mods);
		}

		[[nodiscard]] bool isActivated() const{ return activated; }

		void activate() noexcept{ activated = true; }

		void deactivate() noexcept{ activated = false; }
	};

}
