// ReSharper disable CppHidingFunction
export module Core.Input;

import std;

import Geom.Vector2D;
import ext.Concepts;
import OS.Ctrl.Bind.Constants;
import OS.Ctrl.Bind;
import OS.InputListener;
import OS.ApplicationListener;

using namespace OS;

export namespace Core{
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

		void registerBind(InputBind&& bind){
			registerList.registerBind(std::move(bind));
		}

		template <Concepts::Invokable<void()> Func>
		void registerBind(const int key, const int expectedState, const int expectedMode, Func&& func){
			registerBind(InputBind{key, expectedState, expectedMode, std::forward<Func>(func)});
		}

		template <Concepts::Invokable<void()> Func>
		void registerBind(const int key, const int expectedState, Func&& func){
			registerBind(InputBind{key, expectedState, std::forward<Func>(func)});
		}

		template <Concepts::Invokable<void()> Func1, Concepts::Invokable<void()> Func2>
		void registerBind(const int key, const int expectedMode, Func1&& onPress, Func2&& onRelease){
			registerBind(InputBind{key, Ctrl::Act::Press, expectedMode, std::forward<Func1>(onPress)});
			registerBind(InputBind{
					key, Ctrl::Act::Release, expectedMode, std::forward<Func2>(onRelease)
				});
		}

		template <Concepts::Invokable<void()> Func>
		void registerBind(std::initializer_list<std::pair<int, int>> pairs, Func&& func){
			for(auto [key, expectedState] : pairs){
				registerBind(InputBind{key, expectedState, std::forward<Func>(func)});
			}
		}

		template <Concepts::Invokable<void()> Func>
		void registerBind(std::initializer_list<std::pair<int, int>> pairs, const int expectedMode, Func&& func){
			for(auto [key, expectedState] : pairs){
				registerBind(InputBind{key, expectedState, expectedMode, std::forward<Func>(func)});
			}
		}

		template <Concepts::Invokable<void()> Func>
		void registerBind(std::initializer_list<std::tuple<int, int, int>> params, Func&& func){
			for(auto [key, expectedState, expectedMode] : params){
				registerBind(InputBind{key, expectedState, expectedMode, std::forward<Func>(func)});
			}
		}

		[[nodiscard]] bool isPressedKey(const int key) const{
			return registerList.hasAction(key);
		}

		void clearAllBinds(){
			registerList.clear();
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

	class Input final : public ApplicationListener{
	public:
		using PosListener = std::function<void(float, float)>;
		std::vector<PosListener> scrollListeners{};
		std::vector<PosListener> cursorMoveListeners{};
		std::vector<PosListener> velocityListeners{};

		std::vector<InputListener*> inputKeyListeners{};
		std::vector<InputListener*> inputMouseListeners{};

		InputBindGroup binds{};

	protected:
		bool isInbound{false};

		Geom::Vec2 mousePos{};
		Geom::Vec2 lastMousePos{};
		Geom::Vec2 mouseVelocity{};
		Geom::Vec2 scrollOffset{};

		std::vector<InputBindGroup*> subInputs{};

	public:
		/**
		 * @return false if Notfound
		 */
		bool activeBinds(const InputBindGroup* binds){
			if(const auto itr = std::ranges::find(subInputs, binds); itr != subInputs.end()){
				itr.operator*()->activate();
				return true;
			}
			return false;
		}

		/**
		* @return false if Notfound
		*/
		bool deactiveBinds(const InputBindGroup* binds){
			if(const auto itr = std::ranges::find(subInputs, binds); itr != subInputs.end()){
				itr.operator*()->deactivate();
				return true;
			}
			return false;
		}

		void registerSubInput(InputBindGroup* input){
			subInputs.push_back(input);
		}

		void eraseSubInput(InputBindGroup* input){
			std::erase(subInputs, input);
		}

		/**
		 * \brief No remove function provided. If the input binds needed to be changed, clear it all and register the new binds by the action table.
		 * this is a infrequent operation so just keep the code clean.
		 */
		void informMouseAction(const int button, const int action, const int mods){
			binds.informMouseAction(button, action, mods);

			for(const auto& listener : inputMouseListeners){
				listener->inform(button, action, mods);
			}

			for(auto* subInput : subInputs){
				subInput->informMouseAction(button, action, mods);
			}
		}

		void informKeyAction(const int key, const int scanCode, const int action, const int mods){
			binds.informKeyAction(key, action, mods);

			for(const auto& listener : inputKeyListeners){
				listener->inform(key, action, mods);
			}

			for(auto* subInput : subInputs){
				subInput->informKeyAction(key, action, mods);
			}
		}

		void cursorMoveInform(const float x, const float y){
			mousePos.set(x, y);

			for(auto& listener : cursorMoveListeners){
				listener(x, y);
			}
		}

		[[nodiscard]] Geom::Vec2 getCursorPos() const{
			return mousePos;
		}

		[[nodiscard]] Geom::Vec2 getScrollOffset() const{
			return scrollOffset;
		}

		void setScrollOffset(const float x, const float y){
			scrollOffset.set(-x, y);

			for(auto& listener : scrollListeners){
				listener(-x, y);
			}
		}

		[[nodiscard]] bool cursorInbound() const{
			return isInbound;
		}

		void setInbound(const bool b){
			isInbound = b;
		}

		void updatePost(const float delta) override{
			binds.update(delta);

			mouseVelocity = mousePos;
			mouseVelocity -= lastMousePos;
			mouseVelocity /= delta;

			lastMousePos = mousePos;

			for(auto& listener : velocityListeners){
				listener(mouseVelocity.x, mouseVelocity.y);
			}

			for(auto* subInput : subInputs){
				subInput->update(delta);
			}
		}
	};
}
