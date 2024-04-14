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

	public:
		std::function<void(bool)> call{};

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

		void setCall(Concepts::Invokable<void(bool)> auto&& func){
			call = std::forward<decltype(func)>(func);
		}

		void drawStyle() const override{
			Widget::drawStyle();
		}
	};


}
