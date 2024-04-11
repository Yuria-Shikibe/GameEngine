module;

export module UI.Elem;

export import UI.Flags;
export import UI.Align;
export import UI.Action;
import Event;
import Math;
import Geom.Vector2D;
import Graphic.Color;
import Geom.Rect_Orthogonal;
import RuntimeException;

import std;

export namespace UI {
	struct ElemDrawer;
	class Group;
	class Table;
	class Root;

	struct HoverTableBuilder{
		std::function<void(Table&)> builder{};
		float minHoverTime = 25.0f;
		bool useStaticTime = true;
		bool followCursor = false;
		Geom::Vec2 offset{};

		[[nodiscard]] explicit operator bool() const {
			return static_cast<bool>(builder);
		}
	};
}

export namespace UI {
	struct LayoutCell;

	using Rect = Geom::OrthoRectFloat;

	class Elem {
	public:
		int PointCheck{0};
		virtual ~Elem(){
			setUnfocused();
		}

		[[nodiscard]] Elem(){
			Elem::applyDefDrawer();
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
		std::unordered_set<Elem*> focusTarget{};

		Event::EventManager inputListener{
			Event::indexOf<UI::MouseActionPress>(),
			Event::indexOf<UI::MouseActionRelease>(),
			Event::indexOf<UI::MouseActionDrag>(),
			Event::indexOf<UI::MouseActionDoubleClick>(),
			Event::indexOf<UI::MouseActionScroll>(),
			Event::indexOf<UI::CurosrInbound>(),
			Event::indexOf<UI::CurosrExbound>(),
		};

		TouchbilityFlags touchbility = TouchbilityFlags::enabled;

		std::function<bool()> visibilityChecker{nullptr};
		std::function<bool()> disableChecker{nullptr};

		mutable bool layoutChanged{false};

		bool fillParentX{false};
		bool fillParentY{false};
		bool endRow{false};
		bool visiable{true};
		bool requiresLayout{false};
		bool pressed{false};
		bool disabled{false};

		bool quitInboundFocus = true;

		Geom::Vec2 absoluteSrc{};
		Geom::Vec2 minimumSize{};

		ElemDrawer* drawer{nullptr};

		Align::Spacing border{};

		std::queue<std::unique_ptr<Action<Elem>>> actions{};

		ChangeSignal lastSignal{ChangeSignal::notifyNone};

		void clampTargetWidth(float& w) const{
			w = Math::max(w, minimumSize.x);
		}

		void clampTargetHeight(float& h) const{
			h = Math::max(h, minimumSize.y);
		}

		Table* hoverTableHandle{nullptr};
		HoverTableBuilder hoverTablebuilder{};

	public:
		std::string name{"undefind"};

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

		[[nodiscard]] virtual bool isVisiable() const {return visiable;}

		//TODO rename this shit
		[[nodiscard]] constexpr bool needSetMouseUnfocusedAtCursorOutOfBound() const {return quitInboundFocus;}

		[[nodiscard]] constexpr bool isPressed() const {return pressed;}

		/**
		 * @param elem Element To Fill This(it's parent)
		 * @return Expected Bound Of This Child Element
		 */
		virtual Rect getFilledChildrenBound(Elem* elem) const {
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
		virtual void drawBase() const;

		virtual void draw() const;

		virtual void drawContent() const {}

		virtual void setVisible(const bool val) {
			visiable = val;
		}

		virtual void setRoot(Root* const root) {
			this->root = root;
		}

		virtual void applyDefDrawer();

		void setHoverTableBuilder(const HoverTableBuilder& hoverTableBuilder){
			this->hoverTablebuilder = hoverTableBuilder;
		}

		[[nodiscard]] const HoverTableBuilder& getHoverTableBuilder() const{ return hoverTablebuilder; }

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

		void setDrawer(ElemDrawer* drawer);

		void setEmptyDrawer();

		/**
		 * @return The former parent group
		 */
		Group* setParent(Group* parent);

		virtual void callRemove();

		[[nodiscard]] constexpr bool isEndingRow() const {return endRow;}

		constexpr void setEndRow(const bool end) {endRow = end;}

		[[nodiscard]] const std::unordered_set<Elem*>& getFocus() const {return focusTarget;}

		[[nodiscard]] std::unordered_set<Elem*>& getFocus() {return focusTarget;}

		virtual void addFocusTarget(Elem* const target) {
			focusTarget.insert(target);
		}

		virtual void removeFocusTarget(Elem* const target) {
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
			clampTargetWidth(w);
			if(Math::equal(bound.getWidth(), w))return;
			bound.setWidth(w);
			changed(UI::ChangeSignal::notifyAll);
		}

		virtual void setHeight(float h) {
			clampTargetHeight(h);
			if(Math::equal(bound.getHeight(), h))return;
			bound.setHeight(h);
			changed(UI::ChangeSignal::notifyAll);
		}

		void setSize(float w, float h) {
			clampTargetWidth(w);
			clampTargetHeight(h);
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

		virtual void calAbsoluteSrc(Elem* parent) {
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

		virtual void update(const float delta){
			if(visibilityChecker)visiable = visibilityChecker();
			if(disableChecker)setDisabled(disableChecker());

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

		template <Concepts::Derived<Action<Elem>> ActionType, typename ...T>
		void pushAction(T... args){
			actions.push(std::make_unique<ActionType>(args...));
		}

		template<Concepts::Derived<Action<Elem>> ...ActionType>
		void pushActions(std::unique_ptr<ActionType>&& ...actionArgs){
			std::array<Action<Elem>, sizeof...(actionArgs)> arr = {actionArgs...};
			actions.push_range(arr);
		}

		virtual const std::vector<std::unique_ptr<Elem>>* getChildren() const {return nullptr;}

		virtual bool hasChildren() const {return false;}

		[[nodiscard]] constexpr bool isInteractable() const {return touchbility == TouchbilityFlags::enabled && visiable;}

		virtual bool hintInbound_validToParent(const Geom::Vec2 screenPos){
			return isInbound(screenPos);
		}

		virtual bool isInbound(Geom::Vec2 screenPos);

		bool isFocusedKeyInput() const;

		bool isFocusedScroll() const;

		bool isCursorInbound() const;

		void setFocusedKey(bool focus) const;

		void setFocusedScroll(bool focus);

		void setUnfocused() const;

		[[nodiscard]] constexpr float drawSrcX() const {return absoluteSrc.x;}

		[[nodiscard]] constexpr float drawSrcY() const {return absoluteSrc.y;}

		[[nodiscard]] constexpr float getWidth() const {return bound.getWidth();}

		[[nodiscard]] constexpr float getHeight() const {return bound.getHeight();}

		[[nodiscard]] Align::Spacing getBorder() const{ return border; }

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

	protected:
		virtual void childrenCheck(const Elem* ptr) {
			throw ext::IllegalArguments{"Labels shouldn't have children!"};
		}
	};
}
