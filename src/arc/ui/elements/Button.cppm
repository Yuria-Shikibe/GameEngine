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
		void trigger() const{
			if(call)call();
		}

	public:
		std::function<void()> call{};

		Button(){
			touchbility = TouchbilityFlags::enabled;

			inputListener.on<UI::MouseActionPress>([this](const auto& event) {
				pressed = true;
			});

			inputListener.on<UI::MouseActionRelease>([this](const auto& event) {
				pressed = false;
				if(this->isInbound(event)){
					trigger();
				}
			});

			inputListener.on<UI::CurosrExbound>([this](const auto& event) {
				pressed = false;
			});
		}


		void setCall(Concepts::Invokable<void()> auto&& func){
			this->call = func;
		}

		void drawBackground() const override{
			Elem::drawBackground();
		}
	};


}
