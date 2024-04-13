module;

export module UI.SliderBar;

export import UI.Elem;
export import UI.Root;

import std;
import Math;
import Graphic.Draw;
import Graphic.Color;
import Geom.Rect_Orthogonal;

export namespace UI{
	struct SliderBarDrawer;

	class SliderBar : public Widget{
	protected:
		SliderBarDrawer* barDrawer{nullptr};
		Geom::Vec2 barLastSize{10.0f, 10.0f};
		/**
		 * @brief Has 2 degree of freedom [x, y]
		 */
		Geom::Vec2 barProgress{};
		Geom::Vec2 barLastProgress{};

		[[nodiscard]] Geom::Vec2 getBarTotalPos() const{
			return Geom::Vec2{getValidWidth(), getValidHeight()} - barBaseSize;
		}

		[[nodiscard]] Geom::Vec2 getSegmentUnit() const{
			return getBarTotalPos() / segments.copy().maxX(1).maxY(1).as<float>();
		}

		void moveBar(const Geom::Vec2 movement){
			if(isSegmentMoveActivated()){
				barLastProgress = (barProgress + (movement * slideSensitivity).round(getSegmentUnit()) / getBarTotalPos()).clampNormalized();
			}else{
				barLastProgress = (barProgress + (movement * slideSensitivity) / getBarTotalPos()).clampNormalized();
			}
		}

		void applyLast(){
			if(barProgress != barLastProgress){
				barProgress = barLastProgress;
				if(onChange){
					onChange(barProgress);
				}
			}
		}

		void resumeLast(){
			barLastProgress = barProgress;
		}

	public:
		/**
		 * @brief set 0 to disable one freedom degree
		 * Negative value is accepted to flip the operation
		 */
		Geom::Vec2 slideSensitivity{1.0f, 1.0f};
		/**
		 * @brief Negative value is accepted to flip the operation
		 */
		Geom::Vec2 scrollSensitivity{6.0f, 3.0f};

		Geom::Vec2 barBaseSize{10.0f, 10.0f};

		Geom::Point2U segments{};

		SliderBar(){
			inputListener.on<UI::MouseActionPress>([this](const auto& event) {
				pressed = true;
			});

			inputListener.on<UI::MouseActionScroll>([this](const auto& event) {
				Geom::Vec2 move = event;

				if(isSegmentMoveActivated()){
					move.normalizeToBase().mul(getSegmentUnit());
				}

				if(isClamped()){
					moveBar(scrollSensitivity * slideSensitivity.normalizeToBase() * move.y);
				}else{
					moveBar(move * scrollSensitivity);
				}

				applyLast();
			});

			inputListener.on<UI::MouseActionDrag>([this](const UI::MouseActionDrag& event) {
				moveBar(event.getRelativeMove());
			});

			inputListener.on<UI::MouseActionRelease>([this](const auto& event) {
				pressed = false;
				applyLast();
			});

			inputListener.on<UI::CurosrExbound>([this](const auto& event) {
				pressed = false;
				resumeLast();
				Widget::setFocusedScroll(false);
			});

			inputListener.on<UI::CurosrInbound>([this](const auto& event) {
				Widget::setFocusedScroll(true);
			});

			quitInboundFocus = false;
			touchbility = UI::TouchbilityFlags::enabled;
			// segments.set(8, 1);

			SliderBar::applyDefDrawer();
		}

		/**
		 * @brief return true if the state is invalid
		 * TODO ret necessity
		 */
		std::function<void(Geom::Vec2)> onChange{nullptr};

		void setDefProgress(const Geom::Vec2 progress){
			this->barProgress = progress;
		}

		void setSegments(const Geom::Point2U seg){
			segments = seg;
		}

		void setSliderSize(const Geom::Vec2 sizeVec){
			barBaseSize = sizeVec;
		}

		[[nodiscard]] bool isSegmentMoveActivated() const{
			return segments.x | segments.y;
		}

		[[nodiscard]] bool isClamped() const{
			return isClampedOnHori() || isClampedOnVert();
		}
		
		[[nodiscard]] Geom::Vec2 getBarLastPos() const{
			return getBarTotalPos() * barLastProgress;
		}
		
		[[nodiscard]] Geom::Vec2 getBarCurPos() const{
			return getBarTotalPos() * barProgress;
		}

		[[nodiscard]] Geom::Vec2 getProgress() const{
			return barProgress;
		}

		[[nodiscard]] Geom::Vec2 getBarDrawSize() const{ return barLastSize; }
		
		void setClampedOnHori(){slideSensitivity.y = 0.0f;}

		[[nodiscard]] bool isClampedOnHori() const{return Math::zero(slideSensitivity.y);}

		void setClampedOnVert(){slideSensitivity.x = 0.0f;}

		[[nodiscard]] bool isClampedOnVert() const{return Math::zero(slideSensitivity.x);}


		void update(const float delta) override{
			if(isClampedOnVert()){
				barLastSize.x = getValidWidth();
			}else{
				barLastSize.x = barBaseSize.x;
			}

			if(isClampedOnHori()){
				barLastSize.y = getValidHeight();
			}else{
				barLastSize.y = barBaseSize.y;
			}

			Widget::update(delta);
		}

		void applyDefDrawer() override;

		void drawContent() const override;
	};
}
