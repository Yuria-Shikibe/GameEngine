module;

export module UI.Elem;

import UI.Flags;
import Event;
import Geom.Vector2D;
import Graphic.Color;
import Geom.Shape.Rect_Orthogonal;
import RuntimeException;

import std;

export namespace UI {
	struct ElemDrawer;
	class Group;
	class Root;
}

export namespace UI {
	using Rect = Geom::Shape::OrthoRectFloat;

	//TODO fuck this bug!

	class Elem {
	public:
		virtual ~Elem();

		// ReSharper disable once CppFunctionIsNotImplemented
		[[nodiscard]] Elem();

	protected:
		/**
		 * \brief The srcx, srcy is relative to its parent.
		 */
		Rect bound{};
		Group* parent{nullptr};
		mutable ::UI::Root* root{nullptr};

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

		mutable bool layoutChanged{false};

		bool fillParentX{false};
		bool fillParentY{false};
		bool endRow{false};
		bool visiable{true};
		bool requiresLayout{false};
		bool pressed = false;

		bool quitInboundFocus = true;

		Geom::Vec2 absoluteSrc{};

		ElemDrawer* drawer{nullptr};

	public:
		std::string name{"undefind"};

		Graphic::Color color{1.0f, 1.0f, 1.0f, 1.0f};
		mutable Graphic::Color tempColor{0.0f, 0.0f, 0.0f, 0.0f};
		mutable float maskOpacity = 1.0f;
		std::any animationData{nullptr};

		Geom::Vec2 margin_bottomLeft{};
		Geom::Vec2 margin_topRight{};

		[[nodiscard]] virtual bool isVisiable() const {
			return visiable;
		}

		[[nodiscard]] bool quitMouseFocusAtOutbound() const {
			return quitInboundFocus;
		}

		[[nodiscard]] bool isPressed() const {
			return pressed;
		}

		virtual Rect getFilledChildrenBound(Elem* elem) const {
			Rect bound = elem->bound;

			if(elem->fillParentX){
				bound.setSrcX(margin_bottomLeft.x);
				bound.setWidth(getWidth() - marginWidth());
			}

			if(elem->fillParentY){
				bound.setSrcY(margin_bottomLeft.y);
				bound.setHeight(getHeight() - marginHeight());
			}

			return bound;
		}

		virtual bool layout_fillParent();

		[[nodiscard]] bool isFillParentX() const {
			return fillParentX;
		}

		[[nodiscard]] bool isFillParentY() const {
			return fillParentY;
		}

		virtual void layout() {
			layout_fillParent();

			layoutChanged = false;
		}

		virtual void applySettings() {
		}

		virtual bool ignoreLayout() const {
			return !visiable;
		}

		virtual void draw() const;

		virtual void drawContent() const {

		}

		virtual void setVisible(const bool val) {
			visiable = val;
		}

		virtual void setRoot(Root* const root) {
			this->root = root;
		}

		void setFillparentX(const bool val = true) {
			if(val != fillParentX) {
				changed();
			}
			fillParentX = val;
		}

		void setFillparentY(const bool val = true) {
			if(val != fillParentY) {
				changed();
			}
			fillParentY = val;
		}

		void setFillparent(const bool valX = true, const bool valY = true) {
			setFillparentX(valX);
			setFillparentY(valY);
		}

		[[nodiscard]] bool touchDisabled() const {
			return touchbility == TouchbilityFlags::disabled;
		}

		[[nodiscard]] TouchbilityFlags getTouchbility() const {
			return touchbility;
		}

		void setTouchbility(const TouchbilityFlags flag) {
			this->touchbility = flag;
		}

		float marginHeight() const {
			return margin_bottomLeft.y + margin_topRight.y;
		}

		float marginWidth() const {
			return margin_bottomLeft.x + margin_topRight.x;
		}

		virtual void drawBackground() const;

		[[nodiscard]] Group* getParent() const;

		void setDrawer(ElemDrawer* drawer);

		Group* setParent(Group* parent);

		virtual Elem& prepareRemove();

		[[nodiscard]] bool endingRow() const {
			return endRow;
		}

		void setEndRow(const bool end) {
			endRow = end;
		}

		[[nodiscard]] const std::unordered_set<Elem*>& getFocus() const {
			return focusTarget;
		}

		[[nodiscard]] std::unordered_set<Elem*>& getFocus() {
			return focusTarget;
		}

		virtual void addFocusTarget(Elem* const target) {
			focusTarget.insert(target);
		}

		virtual void removeFocusTarget(Elem* const target) {
			focusTarget.erase(target);
		}

		void setSrc(const float x, const float y) {
			if(bound.getSrcX() == x && bound.getSrcY() == y)return;
			bound.setSrc(x, y);
			changed();
		}

		virtual void setWidth(const float w) {
			if(bound.getWidth() == w)return;
			bound.setWidth(w);
			changed();
		}

		virtual void setHeight(const float h) {
			if(bound.getHeight() == h)return;
			bound.setHeight(h);
			changed();
		}

		virtual void setSize(const float w, const float h) {
			if(bound.getWidth() == w && bound.getHeight() == h)return;
			bound.setSize(w, h);
			changed();
		}

		virtual void setSize(const float s) {
			if(bound.getWidth() == s && bound.getHeight() == s)return;
			bound.setSize(s, s);
			changed();
		}

		virtual float getIdealWidth() {
			return bound.getWidth();
		}

		virtual float getIdealHeight() {
			return bound.getHeight();
		}

		[[nodiscard]] const Rect& getBound() const {
			return bound;
		}

		[[nodiscard]] Rect& getBound() {
			return bound;
		}

		virtual void calAbsolute(Elem* parent) {
			Geom::Vec2 vec{parent->absoluteSrc};
			vec.add(bound.getSrcX(), bound.getSrcY());
			if(vec == absoluteSrc)return;
			absoluteSrc.set(vec);
		}

		Geom::Vec2& getAbsSrc() {
			return absoluteSrc;
		}

		virtual int elemSerializationID() {
			return 0;
		}
		[[nodiscard]] Event::EventManager& getInputListener() {
			return inputListener;
		}

		[[nodiscard]] const Event::EventManager& getInputListener() const {
			return inputListener;
		}

		[[nodiscard]] bool hasChanged() const {
			return layoutChanged;
		}

		virtual void changed() const;

		void toString(std::ostream& os, const int depth) const {
			//TODO tree print support
		}

		virtual void update(float delta){

		}

		virtual std::vector<std::unique_ptr<Elem>>* getChildren() {
			return nullptr;
		}

		virtual bool hasChildren() const {
			return false;
		}

		bool interactive() const {
			return touchbility == TouchbilityFlags::enabled && visiable;
		}

		virtual bool inbound_validToParent(const Geom::Vec2& screenPos) const {
			return inbound(screenPos);
		}

		virtual bool inbound(const Geom::Vec2& screenPos) const;

		bool isFocusedKey() const;

		bool isFocusedScroll() const;

		bool cursorInbound() const;

		void setFocusedKey(bool focus) const;

		void setFocusedScroll(bool focus) const;

		void setUnfocused() const;

		[[nodiscard]] float drawSrcX() const {
			return absoluteSrc.x;
		}

		[[nodiscard]] float drawSrcY() const {
			return absoluteSrc.y;
		}

		[[nodiscard]] float getWidth() const {
			return bound.getWidth();
		}

		[[nodiscard]] float getHeight() const {
			return bound.getHeight();
		}

		void setMarginZero() {
			margin_bottomLeft.setZero();
			margin_topRight.setZero();
		}

		void setMargin(const float left, const float right, const float top, const float bottom) {
			margin_bottomLeft.set(left, bottom);
			margin_topRight.set(right, top);
		}

		void setMargin(const float val) {
			margin_bottomLeft.set(val, val);
			margin_topRight.set(val, val);
		}

		virtual void childrenCheck(const Elem* ptr) {
#ifdef  _DEBUG
			if(!ptr)throw ext::NullPointerException{"Empty Elem"};
#else
			return;
#endif
		}
	};
}
