module;

export module UI.SliderBar;

export import UI.Elem;
export import UI.Root;

import std;
import Math;
import Graphic.Draw;
import Graphic.Color;
import Geom.Shape.Rect_Orthogonal;

export namespace UI{
	class SliderBar : public Elem{
	protected:
		Geom::Vec2 barBaseSize{10.0f, 10.0f};

		Geom::Vec2 barTempSize{10.0f, 10.0f};
		/**
		 * @brief Has 2 degree of freedom [x, y]
		 */
		Geom::Vec2 barProgress{};

		Geom::Vec2 barTempProgress{};
		/**
		 * @brief set 0 to disable one freedom degree
		 */
		Geom::Vec2 slideSensitivity{1.0f, 1.0f};

		Geom::Point2 segments{};

		/**
		 * @brief return true if the state is invalid
		 * TODO ret necessity
		 */
		std::function<void(Geom::Vec2)> onChange{nullptr};

		[[nodiscard]] Geom::Vec2 getBarTotalPos() const{
			return Geom::Vec2{getValidWidth(), getValidHeight()} - barBaseSize;
		}

		[[nodiscard]] Geom::Vec2 getBarTempPos() const{
			return getBarTotalPos() * barTempProgress;
		}

		[[nodiscard]] Geom::Vec2 getBarCurPos() const{
			return getBarTotalPos() * barProgress;
		}

		void moveBar(const Geom::Vec2 movement){
			std::cout << movement << std::endl;
			if(this->activeSegmentMove()){
				barTempProgress = (barProgress + (movement * slideSensitivity).round(getBarTotalPos() / this->segments.maxX(1).maxY(1).as<float>()) / getBarTotalPos()).clampNormalized();
			}else{
				barTempProgress = (barProgress + (movement * slideSensitivity) / getBarTotalPos()).clampNormalized();

			}
		}

		void applyTemp(){
			if(barProgress != barTempProgress){
				barProgress = barTempProgress;
				if(this->onChange){
					onChange(barProgress);
				}
			}

		}

		void resumeTemp(){
			barTempProgress = barProgress;
		}

	public:
		SliderBar(){
			inputListener.on<UI::MouseActionPress>([this](const auto& event) {
				pressed = true;
			});

			inputListener.on<UI::MouseActionDrag>([this](const auto& event) {
				this->moveBar(event.relativeMove);
			});

			inputListener.on<UI::MouseActionRelease>([this](const auto& event) {
				pressed = false;
				applyTemp();
			});

			inputListener.on<UI::CurosrExbound>([this](const auto& event) {
				pressed = false;
				resumeTemp();
			});

			getInputListener().on<UI::MouseActionPress>([this](const auto& e){
				// switch(e.buttonID){
				//
				// }
			});

			quitInboundFocus = false;

			// segments.set(8, 1);
		}

		[[nodiscard]] bool activeSegmentMove() const{
			return !segments.isZero();
		}

		[[nodiscard]] Geom::Vec2 getProgress() const{
			return barProgress;
		}

		void setClampedOnHori(){slideSensitivity.y = 0.0f;}

		[[nodiscard]] bool isClampedOnHori() const{return Math::zero(slideSensitivity.y);}

		void setClampedOnVert(){slideSensitivity.x = 0.0f;}

		[[nodiscard]] bool isClampedOnVert() const{return Math::zero(slideSensitivity.x);}


		void update(float delta) override{
			barTempSize = barBaseSize;

			if(isClampedOnVert()){
				barTempSize.x = getValidWidth();
			}

			if(isClampedOnHori()){
				barTempSize.y = getValidHeight();
			}
			// setWidth(getWidth() + 2.0f);

			setClampedOnHori();
		}

		void drawContent() const override{
			Rect rect{};

			rect.setSrc(border.bot_lft()).setSize(getValidWidth(), getValidHeight());
			rect.move(absoluteSrc);

			using namespace Graphic;
			Draw::color(Colors::LIGHT_GRAY);
			Draw::alpha(0.15f + (isPressed() ? 0.1f : 0.0f));
			Draw::rectOrtho(Draw::defaultTexture, rect);
			Draw::alpha();

			Draw::color(Colors::GRAY);
			rect.setSize(barTempSize);
			rect.setSrc(absoluteSrc + border.bot_lft() + getBarCurPos());
			Draw::rectOrtho(Draw::defaultTexture, rect);

			Draw::color(Colors::LIGHT_GRAY);
			rect.setSize(barTempSize);
			rect.setSrc(absoluteSrc + border.bot_lft() + getBarTempPos());
			Draw::rectOrtho(Draw::defaultTexture, rect);
		}
	};
}
