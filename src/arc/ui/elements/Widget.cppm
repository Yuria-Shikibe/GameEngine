module;

export module UI.Widget;

export import UI.Flags;
export import UI.Align;
export import UI.Action;
export import UI.CursorType;
import UI.SeperateDrawable;
import Event;
import Math;
import Geom.Vector2D;
import Graphic.Color;
import Geom.Rect_Orthogonal;
import ext.RuntimeException;

import std;

export namespace UI {
	struct WidgetDrawer;
	class Group;
	class Table;
	class Root;

	constexpr float DisableAutoTooltip = -1.0f;

	struct TooltipBuilder{
		enum struct FollowTarget : unsigned char{
			none,
			cursor,
			parent,
		};

		FollowTarget followTarget{FollowTarget::none};
		float minHoverTime{25.0f};
		bool useStaticTime{true};
		bool autoRelease{true};
		Geom::Vec2 offset{};
		Align::Mode followTargetAlign{Align::Mode::bottom_left};
		Align::Mode tooltipSrcAlign{Align::Mode::top_left};

		std::function<void(Table&)> builder{};

		[[nodiscard]] explicit operator bool() const {
			return static_cast<bool>(builder);
		}

		[[nodiscard]] bool autoBuild() const{
			return minHoverTime > 0.0f;
		}
	};

	using FollowTarget = TooltipBuilder::FollowTarget;
}

export namespace UI {
	struct LayoutCell;

	using Rect = Geom::OrthoRectFloat;

	class Widget : public SeperateDrawable{
	public:
		int PointCheck{0};

		~Widget() override{
			releaseAllFocus();
		}

		[[nodiscard]] Widget(){
			Widget::applyDefDrawer();
		}
	protected:
		/**
		 * \brief The srcx, srcy is relative to its parent.
		 */
		Rect bound{};
		friend class ::UI::LayoutCell;

		Group* parent{nullptr};
		mutable ::UI::Root* root{nullptr};

		//TODO is this necessary?
		std::unordered_set<Widget*> focusTarget{};

		Event::EventManager inputListener{
			Event::indexOf<UI::MouseActionPress>(),
			Event::indexOf<UI::MouseActionRelease>(),
			Event::indexOf<UI::MouseActionDrag>(),
			Event::indexOf<UI::MouseActionDoubleClick>(),
			Event::indexOf<UI::MouseActionScroll>(),
			Event::indexOf<UI::CurosrInbound>(),
			Event::indexOf<UI::CurosrExbound>(),
		};

		TouchbilityFlags touchbility = TouchbilityFlags::disabled;

		mutable bool layoutChanged{false};

		bool fillParentX{false};
		bool fillParentY{false};
		bool endRow{false};
		bool visiable{true};
		bool requiresLayout{false};
		bool pressed{false};
		bool disabled{false};
		bool activated{false};
		bool sleep{false};

		bool quitInboundFocus = true;

		Geom::Vec2 absoluteSrc{};
		Geom::Vec2 minimumSize{};
		Geom::Vec2 maximumSize{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
		//TODO should here be a maximum size

		WidgetDrawer* drawer{nullptr};

		Align::Spacing border{};

		std::queue<std::unique_ptr<Action<Widget>>> actions{};

		ChangeSignal lastSignal{ChangeSignal::notifyNone};

		[[nodiscard]] float clampTargetWidth(const float w) const{
			return  Math::clamp(w, minimumSize.x, maximumSize.x);
		}

		[[nodiscard]] float clampTargetHeight(const float h) const{
			return  Math::clamp(h, minimumSize.y, maximumSize.y);
		}

		Table* hoverTableHandle{nullptr};
		TooltipBuilder tooltipbuilder{};

	public:
		Widget(const Widget& other) = delete;

		Widget(Widget&& other) noexcept = default;

		Widget& operator=(const Widget& other) = delete;

		Widget& operator=(Widget&& other) noexcept = default;

		std::string name{"undefind"};

		std::function<bool()> visibilityChecker{nullptr};
		std::function<bool()> disableChecker{nullptr};
		std::function<bool()> activatedChecker{nullptr};

		Graphic::Color color{1.0f, 1.0f, 1.0f, 1.0f};

		mutable Graphic::Color tempColor{0.0f, 0.0f, 0.0f, 0.0f};
		mutable float maskOpacity = 1.0f;
		float selfMaskOpacity = 1.0f;

		std::any animationData{};

		[[nodiscard]] Geom::Vec2 getMinimumSize() const{ return minimumSize; }

		void setMinimumSize(const Geom::Vec2 minimumSize){
			this->minimumSize = minimumSize;
			setWidth(Math::max(minimumSize.x, getWidth()));
			setHeight(Math::max(minimumSize.y, getHeight()));
		}


		[[nodiscard]] Geom::Vec2 getMaximumSize() const{ return maximumSize; }

		void setMaximumSize(const Geom::Vec2 maximumSize){
			this->maximumSize = maximumSize;
			setWidth(Math::min(maximumSize.x, getWidth()));
			setHeight(Math::min(maximumSize.y, getHeight()));
		}

		[[nodiscard]] virtual bool isVisiable() const {return visiable;}

		//TODO rename this shit
		[[nodiscard]] constexpr bool shouldDropFocusAtCursorQuitBound() const{
			return quitInboundFocus;
		}

		constexpr void setDropFocusAtCursorQuitBound(const bool quitInboundFocus){ this->quitInboundFocus = quitInboundFocus; }

		[[nodiscard]] constexpr bool isPressed() const {return pressed;}

		[[nodiscard]] bool isSleep() const{ return sleep; }

		void setSleep(const bool sleep){ this->sleep = sleep; }

		/**
		 * @param elem Element To Fill This(it's parent)
		 * @return Expected Bound Of This Child Element
		 */
		virtual Rect getFilledChildrenBound(Widget* elem) const {
			Rect bound = elem->bound;

			bound.setSrc(border.bot_lft());

			if(elem->fillParentX){
				bound.setWidth(getValidWidth());
			}

			if(elem->fillParentY){
				bound.setHeight(getValidHeight());
			}

			return bound;
		}

		virtual bool layout_tryFillParent();

		[[nodiscard]] bool isFillParentX() const {
			return fillParentX;
		}

		[[nodiscard]] bool isFillParentY() const {
			return fillParentY;
		}

		virtual void layout() {
			layout_tryFillParent();

			layoutChanged = false;
		}

		virtual void applySettings() {}

		virtual bool isIgnoreLayout() const {
			return !visiable;
		}

		/**
		 * @brief Used to create blur effect, this is draw as a mask
		 */
		void drawBase() const override;

		void draw() const override;

		virtual void drawContent() const {}

		virtual void setVisible(const bool val) {
			visiable = val;
		}

		virtual void setRoot(Root* const root) {
			this->root = root;
		}

		[[nodiscard]] UI::Root* getRoot() const{ return root; }

		virtual void applyDefDrawer();

		void setTooltipBuilder(const TooltipBuilder& hoverTableBuilder){
			this->tooltipbuilder = hoverTableBuilder;
		}

		void setTooltipBuilder(TooltipBuilder&& hoverTableBuilder){
			this->tooltipbuilder = std::move(hoverTableBuilder);
		}

		[[nodiscard]] const TooltipBuilder& getTooltipBuilder() const{ return tooltipbuilder; }

		void updateHoverTableHandle(Table* handle){
			this->hoverTableHandle = handle;
		}

		void setFillparentX(const bool val = true) {
			if(val != fillParentX)changed(UI::ChangeSignal::notifySubs);

			fillParentX = val;
		}

		void setFillparentY(const bool val = true) {
			if(val != fillParentY)changed(UI::ChangeSignal::notifySubs);

			fillParentY = val;
		}

		void setFillparent(const bool valX = true, const bool valY = true) {
			setFillparentX(valX);
			setFillparentY(valY);
		}

		[[nodiscard]] constexpr bool touchDisabled() const {
			return touchbility == TouchbilityFlags::disabled;
		}

		[[nodiscard]] constexpr TouchbilityFlags getTouchbility() const {
			return touchbility;
		}

		constexpr void setTouchbility(const TouchbilityFlags flag) {
			this->touchbility = flag;
		}

		[[nodiscard]] constexpr float getBorderWidth() const {return border.getWidth();}

		[[nodiscard]] constexpr float getBorderHeight() const {return border.getHeight();}

		[[nodiscard]] constexpr float getValidWidth() const{return Math::clampPositive(getWidth() - getBorderWidth());}

		[[nodiscard]] constexpr float getValidHeight() const{return Math::clampPositive(getHeight() - getBorderHeight());}

		virtual void drawStyle() const;

		[[nodiscard]] Group* getParent() const;

		void setDrawer(WidgetDrawer* drawer);

		virtual void setEmptyDrawer();

		/**
		 * @return The former parent group
		 */
		Group* setParent(Group* parent);

		virtual void callRemove();

		[[nodiscard]] constexpr bool isEndingRow() const {return endRow;}

		constexpr void setEndRow(const bool end) {endRow = end;}

		[[nodiscard]] const std::unordered_set<Widget*>& getFocus() const {return focusTarget;}

		[[nodiscard]] std::unordered_set<Widget*>& getFocus() {return focusTarget;}

		virtual void addFocusTarget(Widget* const target) {
			focusTarget.insert(target);
		}

		virtual void removeFocusTarget(Widget* const target) {
			focusTarget.erase(target);
		}

		void setSrc(const Geom::Vec2 src) {
			setSrc(src.x, src.y);
		}

		void setSrc(const float x, const float y) {
			if(bound.getSrcX() == x && bound.getSrcY() == y)return;
			bound.setSrc(x, y);
			changed(UI::ChangeSignal::notifyAll);
		}

		virtual void setWidth(float w) {
			w = clampTargetWidth(w);
			if(Math::equal(bound.getWidth(), w))return;
			bound.setWidth(w);
			changed(UI::ChangeSignal::notifyAll);
		}

		virtual void setHeight(float h) {
			h = clampTargetHeight(h);
			if(Math::equal(bound.getHeight(), h))return;
			bound.setHeight(h);
			changed(UI::ChangeSignal::notifyAll);
		}

		void setSize(const float w, const float h) {
			setWidth(w);
			setHeight(h);
			changed(UI::ChangeSignal::notifyAll);
		}

		void setSize(const float s) {
			setSize(s, s);
		}

		virtual float getIdealWidth() const noexcept {return bound.getWidth();}

		virtual float getIdealHeight() const noexcept {return bound.getHeight();}

		[[nodiscard]] constexpr Rect getBound() const noexcept {return bound;}

		[[nodiscard]] constexpr Rect getValidBound() const noexcept {return {border.left, border.bottom, getValidWidth(), getValidHeight()};}

		virtual void calAbsoluteSrc(Widget* parent) {
			Geom::Vec2 vec = parent ? parent->absoluteSrc : Geom::ZERO;

			vec.add(bound.getSrc());
			if(vec == absoluteSrc)return;
			absoluteSrc.set(vec);
			calAbsoluteChildren();
		}

		virtual void calAbsoluteChildren() {}

		[[nodiscard]] Geom::Vec2 getAbsSrc() const{
			return absoluteSrc;
		}

		void setAbsSrc(const Geom::Vec2 src){
			if(absoluteSrc != src){
				absoluteSrc = src;
				changed(UI::ChangeSignal::notifyAll);
			}
		}

		virtual int elemSerializationID() {
			return 0;
		}

		[[nodiscard]] Event::EventManager& getInputListener() {return inputListener;}

		[[nodiscard]] const Event::EventManager& getInputListener() const {return inputListener;}

		[[nodiscard]] constexpr bool hasChanged() const {return layoutChanged;}

		/**
		 * @brief
		 * This is a post signal function.
		 * After change is called, layout should be called in the next update to handle the change.
		 */
		void changed(const ChangeSignal direction, const ChangeSignal removal = ChangeSignal::notifyNone){
			lastSignal = lastSignal + direction - removal;
		}

		virtual void postChanged();

		void overrideChanged(const bool val, const ChangeSignal removal = ChangeSignal::notifyNone){
			layoutChanged = val;
			lastSignal = removal;
		}

		void toString(std::ostream& os, const int depth) const {
			//TODO tree print support
		}

		virtual void setDisabled(const bool disabled){
			this->disabled = disabled;
		}

		[[nodiscard]] std::function<bool()>& getActivatedChecker(){ return activatedChecker; }

		void setActivatedChecker(const std::function<bool()>& activatedChecker){
			this->activatedChecker = activatedChecker;
		}

		virtual void setActivated(const bool activated){
			this->activated = activated;
		}

		[[nodiscard]] bool isActivated() const{ return activated; }

		virtual void update(const float delta){
			if(visibilityChecker)setVisible(visibilityChecker());
			if(disableChecker)setDisabled(disableChecker());
			if(activatedChecker)setActivated(activatedChecker());

			float actionDelta = delta;

			while(!actions.empty()){
				const auto& current = actions.front();

				actionDelta = current->update(actionDelta, this);

				if(actionDelta >= 0) [[unlikely]] {
					actions.pop();
				}else{
					break;
				}
			}
		}

		[[nodiscard]] auto& getActions() const{ return actions; }

		template <Concepts::Derived<Action<Widget>> ActionType, typename ...T>
		void pushAction(T... args){
			actions.push(std::make_unique<ActionType>(args...));
		}

		template<Concepts::Derived<Action<Widget>> ...ActionType>
		void pushActions(std::unique_ptr<ActionType>&& ...actionArgs){
			std::array<Action<Widget>, sizeof...(actionArgs)> arr = {actionArgs...};
			actions.push_range(arr);
		}

		virtual const std::vector<std::unique_ptr<Widget>>* getChildren() const {return nullptr;}

		virtual bool hasChildren() const {return false;}

		[[nodiscard]] constexpr bool isInteractable() const{
			return (touchbility == TouchbilityFlags::enabled || static_cast<bool>(tooltipbuilder)) && visiable;
		}

		[[nodiscard]] constexpr bool isQuietInteractable() const{
			return (touchbility != TouchbilityFlags::enabled && static_cast<bool>(tooltipbuilder)) && visiable;
		}

		virtual bool hintInbound_validToParent(const Geom::Vec2 screenPos){
			return isInbound(screenPos);
		}

		[[nodiscard]] virtual bool isInbound(Geom::Vec2 screenPos) const;

		bool isFocusedKeyInput() const;

		bool isFocusedScroll() const;

		bool isCursorInbound() const;

		void setFocusedKey(bool focus) const;

		void setFocusedScroll(bool focus);

		void releaseAllFocus() const;

		virtual CursorType getCursorType() const{
			if(touchDisabled()){
				return tooltipbuilder ? CursorType::regular_tip : CursorType::regular;
			}else{
				return tooltipbuilder ? CursorType::clickable_tip : CursorType::clickable;
			}
		}

		[[nodiscard]] constexpr float drawSrcX() const {return absoluteSrc.x;}

		[[nodiscard]] constexpr float drawSrcY() const {return absoluteSrc.y;}

		[[nodiscard]] constexpr float getWidth() const {return bound.getWidth();}

		[[nodiscard]] constexpr float getHeight() const {return bound.getHeight();}

		[[nodiscard]] const Align::Spacing& getBorder() const{ return border; }

		[[nodiscard]] Align::Spacing& getBorder() { return border; }

		void setBorderZero() {setBorder(0.0f);}

		void setBorder(const Align::Spacing margin) {
			if(margin != this->border){
				this->border = margin;
				changed(UI::ChangeSignal::notifySubs);
			}
		}

		void setBorder(const float val) {
			if(border != val){
				this->border.set(val);
				changed(UI::ChangeSignal::notifySubs);
			}
		}

		bool keyDown(const int code, const int action, const int mode) const;

		/**
		 * @return True if there is nothing can do, then transfer the control
		 */
		virtual bool onEsc(){
			return true;
		}

		void buildTooltip();

	protected:
		virtual void childrenCheck(const Widget* ptr) {
			throw ext::IllegalArguments{"Labels shouldn't have children!"};
		}
	};
}
