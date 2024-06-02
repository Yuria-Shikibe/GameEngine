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
		bool triggerOnReleaseOnly{true};

		//TODO keyCodeSupport
		std::function<void(Button&, bool)> call{};
		Assets::Sounds::SoundSource defClickSound = Assets::Sounds::uiClick;

		void trigger(const bool isButtonPressed){
			if(isButtonPressed)passSound(defClickSound);
			if(call){
				if(triggerOnReleaseOnly){
					if(!isButtonPressed)call(*this, isButtonPressed);
				}else{
					call(*this, isButtonPressed);
				}
			}
		}

	public:
		Button(){
			touchbility = TouchbilityFlags::enabled;

			inputListener.on<MouseActionPress>([this](const auto& event) {
				pressed = true;
				trigger(pressed);
			});

			inputListener.on<MouseActionRelease>([this](const auto& event) {
				pressed = false;
				if(this->isInbound(event)){
					trigger(pressed);
				}
			});

			inputListener.on<CurosrExbound>([this](const auto& event) {
				pressed = false;
			});
		}

		[[nodiscard]] bool isTriggerOnReleaseOnly() const{ return triggerOnReleaseOnly; }

		void setTriggerOnReleaseOnly(const bool triggerOnReleaseOnly){
			this->triggerOnReleaseOnly = triggerOnReleaseOnly;
		}

		void setCall(Concepts::Invokable<void(Button&, bool)> auto&& func){
			call = std::forward<decltype(func)>(func);
		}

		[[nodiscard]] std::function<void(Button&, bool)>& getCall(){ return call; }

		void drawStyle() const override{
			Elem::drawStyle();
		}
	};

	namespace ButtonFunc{
		void buildTooltip(Button& b, bool){
			b.buildTooltip();
		}

		void buildOrDropTooltip(Button& b, bool){
			if(b.hasTooltip()){
				b.dropTooltip(false);
			}else{
				b.buildTooltip();
			}
		}

		void addButtonTooltipCheck(Button& b){
			b.setActivatedChecker([&b]{
				return b.hasTooltip();
			});
		}
	}
}
