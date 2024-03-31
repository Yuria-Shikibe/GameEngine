//
// Created by Matrix on 2024/3/29.
//

export module UI.Button;

export import UI.Table;

import std;
import Concepts;

export namespace UI{
	class Button : public Table{
	protected:
		void trigger(const bool ON_OFF) const{
			if(call)call(ON_OFF);
		}

		bool mousePressed{false};


		[[nodiscard]] bool getPressedCheckState() const {
			if(pressedCheck){
				return pressedCheck();
			}

			return false;
		}

	public:
		std::function<void(bool)> call{};
		std::function<bool()> pressedCheck{};

		Button(){
			touchbility = TouchbilityFlags::enabled;

			inputListener.on<MouseActionPress>([this](const auto& event) {
				mousePressed = true;
				trigger(mousePressed);
			});

			inputListener.on<MouseActionRelease>([this](const auto& event) {
				mousePressed = false;
				if(this->isInbound(event)){
					trigger(mousePressed);
				}
			});

			inputListener.on<CurosrExbound>([this](const auto& event) {
				mousePressed = false;
			});
		}

		void update(const float delta) override{
			if(this->pressedCheck){
				pressed = pressedCheck();
			}else{
				pressed = mousePressed;
			}
		}

		void setCall(Concepts::Invokable<void(bool)> auto&& func){
			call = std::forward<decltype(func)>(func);
		}

		void drawBackground() const override{
			Elem::drawBackground();
		}
	};


}
