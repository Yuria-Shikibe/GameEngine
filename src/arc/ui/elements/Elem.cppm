module;

export module UI.Elem;

export import UI.Flags;
export import UI.Align;
export import UI.Action;
import Event;
import Math;
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
	using Rect = Geom::OrthoRectFloat;

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

		ElemDrawer* drawer{nullptr};

		Align::Spacing border{};

		std::queue<std::unique_ptr<Action<Elem>>> actions{};

	public:
		std::string name{"undefind"};

		Graphic::Color color{1.0f, 1.0f, 1.0f, 1.0f};

		mutable Graphic::Color tempColor{0.0f, 0.0f, 0.0f, 0.0f};
		mutable float maskOpacity = 1.0f;
		float selfMaskOpacity = 1.0f;

		std::any animationData{};

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

			if(elem->fillParentX){
				bound.setSrcX(border.left);
				bound.setWidth(getWidth() - getBorderWidth());
			}

			if(elem->fillParentY){
				bound.setSrcY(border.bottom);
				bound.setHeight(getHeight() - getBorderHeight());
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
		virtual void drawBase(){}

		virtual void draw() const;

		virtual void drawContent() const {}

		virtual void setVisible(const bool val) {
			visiable = val;
		}

		virtual void setRoot(Root* const root) {
			this->root = root;
		}

		void setFillparentX(const bool val = true) {
			if(val != fillParentX)changed();

			fillParentX = val;
		}

		void setFillparentY(const bool val = true) {
			if(val != fillParentY)changed();

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

		[[nodiscard]] constexpr float getBorderWidth() const {return border.getMarginWidth();}

		[[nodiscard]] constexpr float getBorderHeight() const {return border.getMarginHeight();}

		[[nodiscard]] constexpr float getValidWidth() const{return Math::clampPositive(getWidth() - getBorderWidth());}

		[[nodiscard]] constexpr float getValidHeight() const{return Math::clampPositive(getHeight() - getBorderHeight());}

		virtual void drawBackground() const;

		[[nodiscard]] Group* getParent() const;

		void setDrawer(ElemDrawer* drawer);

		/**
		 * @return The former parent group
		 */
		Group* setParent(Group* parent);

		virtual Elem& prepareRemove();

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

		virtual float getIdealWidth() const {return bound.getWidth();}

		virtual float getIdealHeight() const {return bound.getHeight();}

		[[nodiscard]] constexpr Rect getBound() const {return bound;}

		[[nodiscard]] constexpr Rect& getBoundRef() {return bound;}

		virtual void calAbsoluteSrc(Elem* parent) {
			Geom::Vec2 vec{parent->absoluteSrc};
			vec.add(bound.getSrcX(), bound.getSrcY());
			if(vec == absoluteSrc)return;
			absoluteSrc.set(vec);
		}

		[[nodiscard]] Geom::Vec2 getAbsSrc() const{
			return absoluteSrc;
		}

		void setAbsSrc(Geom::Vec2 src){
			if(absoluteSrc != src){
				absoluteSrc = src;
				changed();
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
		virtual void changed() const;

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

		virtual std::vector<std::unique_ptr<Elem>>* getChildren() {return nullptr;}

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

		[[nodiscard]] Align::Spacing getMargin() const{ return border; }

		void setBorderZero() {setBorder(0.0f);}

		void setBorder(const Align::Spacing margin) {
			if(margin != this->border){
				this->border = margin;
				changed();
			}
		}

		void setBorder(const float val) {
			if(border != val){
				this->border.set(val);
				changed();
			}
		}

	protected:
		virtual void childrenCheck(const Elem* ptr) {
			throw ext::IllegalArguments{"Labels shouldn't have children!"};
		}
	};
}
