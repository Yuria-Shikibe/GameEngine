module;

export module UI.ScrollPane;

import UI.Elem;
import UI.Group;

import Geom.Vector2D;
import Geom.Matrix3D;
import GL;
import std;
import Concepts;

export namespace UI {
	class ScrollPane;

	struct ScrollerDrawer {
		virtual ~ScrollerDrawer() = default;

		virtual void operator()(const ScrollPane* pane) const;
	};

	//TODO move this other place
	std::unique_ptr defScrollBarDrawer{std::make_unique<ScrollerDrawer>()};

	class ScrollPane : public Group {

	protected:
		Geom::Vec2 scrollOffset{};
		Geom::Vec2 scrollTempOffset{};

		Geom::Vec2 scrollVelocity{};
		Geom::Vec2 scrollTargetVelocity{};

		bool usingAccel = true;

		bool enableHorizonScroll_always = false;
		float hoirScrollerHeight{20.0f};

		bool enableVerticalScroll_always = false;
		float vertScrollerWidth{20.0f};

		//TODO fade
		bool fadeWhenUnused = true;
		bool hoverScroller = false;

		float scrollMarginCoefficient = 6.0f;

		float accel = 0.126f;

		ScrollerDrawer* scrollBarDrawer{defScrollBarDrawer.get()};

		Elem* item{nullptr};

		void clamp(Geom::Vec2& offset) const{
			offset.clampX(-Math::max(0.0f, itemSize.getWidth() - getValidWidth() + vertBarStroke()), 0);
			offset.clampY(0, Math::max(0.0f, itemSize.getHeight() - getValidHeight() + horiBarStroke()));
		}

		void move(const Geom::Vec2 offset){
			scrollTempOffset = scrollOffset + offset;

			clamp(scrollTempOffset);
		}

		void applyTemp(){
			scrollOffset = scrollTempOffset;
		}

		void resumeTemp(){
			scrollTempOffset = scrollOffset;
		}

		//TODO uses layout cell
		Rect itemSize{};
	public:
		Geom::Vec2 scrollSensitivity{90.0f, 60.0f};
		ScrollPane(){
			inputListener.on<UI::MouseActionDrag>([this](const UI::MouseActionDrag& event) {
				Geom::Vec2 clamp{Math::num<float>(isInHoriBar(event.begin)), Math::num<float>(isInVertBar(event.begin))};
				move(event.getRelativeMove() * clamp * Geom::Vec2{-itemSize.getWidth() / getValidWidth(), -itemSize.getHeight() / getValidHeight()});
				pressed = true;
			});

			inputListener.on<UI::MouseActionRelease>([this](const auto& event) {
				pressed = false;
				applyTemp();
			});

			inputListener.on<UI::MouseActionScroll>([this](const auto& event) {
				scrollTargetVelocity = event;
				if(ishoriOutOfBound() ^ isvertOutOfBound()){
					scrollTargetVelocity.x = scrollTargetVelocity.y;
				}

				scrollTargetVelocity.scl(-scrollSensitivity.x, -scrollSensitivity.y);
			});

			touchbility = UI::TouchbilityFlags::enabled;
			quitInboundFocus = false;
		}

		void update(const float delta) override {
			scrollVelocity.lerp(scrollTargetVelocity, usingAccel ? (pressed ? 1.0f : Math::clamp(accel * delta)) : 1.0f);

			if(scrollTempOffset != scrollOffset){

			}else{
				scrollOffset.add(scrollVelocity);
				clamp(scrollOffset);
				resumeTemp();
			}

			scrollTargetVelocity.scl(scrollMarginCoefficient).toAbs();

			scrollTargetVelocity.setZero();

			Group::update(delta);

			if(layoutChanged) {
				layout();
			}

			calAbsoluteSrc(parent);
		}

		void calAbsoluteSrc(Elem* parent) override{
			Geom::Vec2 vec{parent->getAbsSrc()};
			vec.add(bound.getSrcX(), bound.getSrcY());
			//TODO scroll offset check
			absoluteSrc.set(vec);

			if(hasChildren()) {
				const Geom::Vec2 absOri = absoluteSrc;

				absoluteSrc += border.bot_lft();
				absoluteSrc += scrollTempOffset;// * Geom::Vec2{Math::num<float>(horiExbound), Math::num<float>(vertExbound)};

				absoluteSrc.y += getValidHeight() - itemSize.getHeight();

				calAbsoluteChildren();

				absoluteSrc = absOri;
			}
		}

		void layout() override {
			Group::layout();

			if(item) {
				itemSize = item->getBound();
			}
		}

		template <Concepts::Derived<Elem> T>
		void setItem(Concepts::Invokable<void(T&)> auto&& func, const int depth = std::numeric_limits<int>::max()) {
			auto ptr = std::make_unique<T>();

			if constexpr (!std::same_as<decltype(func), std::nullptr_t>) {
				func(*ptr);
			}

			getChildren()->clear();
			this->item = ptr.get();
			if(item != nullptr) {
				this->addChildren(std::move(ptr), depth);
				itemSize = item->getBound();
			}
		}

		//TODO this has bug when resized !
		//If layouted multible times, this will shrink itemsize!
		Rect getFilledChildrenBound(Elem* elem) const override {
			Rect rect = Elem::getFilledChildrenBound(elem);

			if(hoverScroller)return rect;

			const bool enableX = !elem->isFillParentX() && elem->getWidth() > getValidWidth();
			const bool enableY = !elem->isFillParentY() && elem->getHeight() > getValidHeight();

			rect.addSize(
				enableY ? -vertScrollerWidth : 0.0f,
				enableX ? -hoirScrollerHeight : 0.0f
			);

			rect.move(
				elem->isFillParentX() && enableY ? -border.left : 0.0f,
				elem->isFillParentY() && enableX ? -border.bottom : 0.0f
			);

			return rect;
		}


		[[nodiscard]] bool hintInbound_validToParent(const Geom::Vec2 screenPos) override {
			return Elem::isInbound(screenPos) && !inbound_scrollBars(screenPos);
		}

		[[nodiscard]] bool inbound_scrollBars(Geom::Vec2 screenPos) const {
			return isInVertBar(screenPos) || isInHoriBar(screenPos);
		}

		[[nodiscard]] bool isInHoriBar(Geom::Vec2 screenPos) const{
			if(!enableHorizonScroll())return false;
			screenPos.y -= absoluteSrc.y + border.bottom;
			return screenPos.y < hoirScrollerHeight;
		}

		[[nodiscard]] bool isInVertBar(Geom::Vec2 screenPos) const{
			if(!enableVerticalScroll())return false;
			screenPos.x -= absoluteSrc.x + border.left;
			return screenPos.x > getValidWidth() - vertScrollerWidth;
		}

		[[nodiscard]] bool isInbound(const Geom::Vec2 screenPos) override {
			if(Elem::isInbound(screenPos)) {
				Elem::setFocusedScroll(true);
				return inbound_scrollBars(screenPos);
			}

			Elem::setFocusedScroll(false);
			return false;
		}

		[[nodiscard]] bool ishoriOutOfBound() const {
			return itemSize.getWidth() > getValidWidth();
		}

		[[nodiscard]] bool isvertOutOfBound() const {
			return itemSize.getHeight() > getValidHeight();
		}

		[[nodiscard]] bool enableVerticalScroll() const {
			return enableVerticalScroll_always || isvertOutOfBound();
		}

		[[nodiscard]] bool enableHorizonScroll() const {
			return enableHorizonScroll_always || ishoriOutOfBound();
		}

		[[nodiscard]] float horiBarStroke() const {
			return  enableHorizonScroll() ? hoirScrollerHeight : 0;
		}

		[[nodiscard]] float vertBarStroke() const {
			return enableVerticalScroll() ? vertScrollerWidth : 0;
		}

		[[nodiscard]] float horiBarLength() const {
			return Math::clampPositive(Math::min(getContentWidth() / itemSize.getWidth(), 1.0f) * getContentWidth());
		}

		[[nodiscard]] float vertBarSLength() const {
			return Math::clampPositive(Math::min(getValidHeight() / itemSize.getHeight(), 1.0f) * getValidHeight());
		}

		/** @return Valid Width - Bar Stroke*/
		[[nodiscard]] float getContentWidth() const {
			return getValidWidth() - vertBarStroke();
		}

		/** @return Valid Height - Bar Stroke*/
		[[nodiscard]] float getContentHeight() const {
			return getValidHeight() - horiBarStroke();
		}

		[[nodiscard]] float horiScrollRatio() const {
			return Math::clamp(-scrollTempOffset.x / (itemSize.getWidth() - getContentWidth()));
		}

		[[nodiscard]] float vertScrollRatio() const {
			return Math::clamp(scrollTempOffset.y / (itemSize.getHeight() - getContentHeight()));
		}

		[[nodiscard]] Rect getHoriBarRect() const {
			return {
				drawSrcX() + border.left + horiScrollRatio() * (getValidWidth() - horiBarLength() - vertBarStroke()),
				drawSrcY() + border.bottom,
				horiBarLength(),
				horiBarStroke()
			};
		}

		[[nodiscard]] Rect getVertBarRect() const {
			return {
				drawSrcX() + getWidth() - border.right - vertBarStroke(),
				drawSrcY() + getHeight() - border.top - vertScrollRatio() * (getValidHeight() - vertBarSLength() - horiBarStroke()),
				vertBarStroke(),
				-vertBarSLength()
			};
		}

		void drawContent() const override;
	};



}

