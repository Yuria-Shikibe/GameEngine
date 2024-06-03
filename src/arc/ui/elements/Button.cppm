module;

export module UI.Button;

export import UI.Table;

import <irrKlang.h>;
import std;
import ext.Concepts;
import Assets.Sound;

export namespace UI{
	class Button : public Table{
	protected:
		std::function<void(Button&, bool, int, int)> call{};
		Assets::Sounds::SoundSource defClickSound = Assets::Sounds::uiClick;

		void trigger(const bool isButtonPressed, const int key, const int mode){
			if(isButtonPressed) passSound(defClickSound);
			if(call){
				call(*this, isButtonPressed, key, mode);
			}
		}

	public:
		Button(){
			touchbility = TouchbilityFlags::enabled;

			inputListener.on<MouseActionPress>([this](const auto& event){
				pressed = true;
				this->trigger(pressed, event.key, event.mode);
			});

			inputListener.on<MouseActionRelease>([this](const auto& event){
				pressed = false;
				if(this->isInbound(event)){
					this->trigger(pressed, event.key, event.mode);
				}
			});

			inputListener.on<CurosrExbound>([this](const auto& event){
				pressed = false;
			});
		}

		template <bool onReleaseOnly = true>
		void setCall(Concepts::Invokable<void()> auto&& func){
			call = [func = std::forward<decltype(func)>(func)](Button&, const bool bo, int, int){
				if constexpr(onReleaseOnly){
					if(!bo) func();
				} else{
					func();
				}
			};
		}

		/**
		 * @param func [button]
		 */
		template <bool onReleaseOnly = true>
		void setCall(Concepts::Invokable<void(Button&)> auto&& func){
			call = [func = std::forward<decltype(func)>(func)](Button& b, const bool bo, int, int){
				if constexpr(onReleaseOnly){
					if(!bo) func(b);
				} else{
					func(b);
				}
			};
		}

		/**
		 * @param func [button, press/release]
		 */
		void setCall(Concepts::Invokable<void(Button&, bool)> auto&& func){
			call = [func = std::forward<decltype(func)>(func)](Button& b, bool bo, int, int){
				func(b, bo);
			};
		}

		/**
		 * @param func [button, press/release, key, mode]
		 */
		void setCall(Concepts::Invokable<void(Button&, bool, int, int)> auto&& func){
			call = std::forward<decltype(func)>(func);
		}

		/**
		 * @param func [button, key, mode]
		 */
		template <bool onReleaseOnly = true>
		void setCall(Concepts::Invokable<void(Button&, int, int)> auto&& func){
			call = [func = std::forward<decltype(func)>(func)](Button& b, const bool bo, int k, int m){
				if constexpr(onReleaseOnly){
					if(!bo) func(b, k, m);
				} else{
					func(b, k, m);
				}
			};
		}

		[[nodiscard]] auto& getCall(){
			return
				call;
		}

		void drawStyle() const override{
			Elem::drawStyle();
		}
	};


	namespace ButtonFunc{
		void buildTooltip(Elem& b){
			b.buildTooltip();
		}

		void buildOrDropTooltip(Elem& b){
			if(b.hasTooltip()){
				b.dropTooltip(false);
			} else{
				b.buildTooltip();
			}
		}

		void addButtonTooltipCheck(Elem& b){
			b.setActivatedChecker([](const UI::Elem& b){
				return b.hasTooltip();
			});
		}

		void setSideMenuBuilder(Elem& b,
			const Align::Layout followTargetAlign,
			const Align::Layout tooltipSrcAlign,
			std::function<void(Table&)>&& builder,
			const bool autoRelease = true
		){
			b.setTooltipBuilder({
					.followTarget = TooltipBuilder::FollowTarget::parent,
					.minHoverTime = UI::DisableAutoTooltip,
					.useStaticTime = false,
					.autoRelease = autoRelease,
					.followTargetAlign = followTargetAlign,
					.tooltipSrcAlign = tooltipSrcAlign,
					.offset = {},
					.builder = std::move(builder)
				});
		}
	}
}
