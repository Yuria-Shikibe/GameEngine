module;

export module UI.Elem;

export import UI.Flags;
import Event;
import Geom.Vector2D;
import Graphic.Color;
import Geom.Shape.Rect_Orthogonal;
import RuntimeException;

import <algorithm>;
import <execution>;
import <functional>;
import <set>;
import <vector>;
import <unordered_set>;

export namespace UI {
	using Rect = Geom::Shape::OrthoRectFloat;

	struct ElemDrawer;
	class Root;

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
		Elem* parent{nullptr};
		mutable ::UI::Root* root{nullptr};

		//TODO abstact this if possible
		std::vector<std::unique_ptr<Elem>> children{};
		std::set<Elem*> toRemove{};

		std::unordered_set<Elem*> focusTarget{};

		Event::EventManager inputListener{
			Event::indexOf<MouseActionPress>(),
			Event::indexOf<MouseActionRelease>(),
			Event::indexOf<MouseActionDrag>(),
			Event::indexOf<MouseActionDoubleClick>(),

			Event::indexOf<MouseActionScroll>(),

			Event::indexOf<CurosrInbound>(),
			Event::indexOf<CurosrExbound>(),
		};

		TouchbilityFlags touchbility = TouchbilityFlags::enabled;

		std::function<bool()> visibilityChecker{nullptr};

		mutable bool layoutChanged{false};

		bool fillParentX{false};
		bool fillParentY{false};
		bool endRow{false};
		bool visiable{true};
		bool requiresLayout{false};

		bool quitInboundFocus = true;

		Geom::Vector2D absoluteSrc{};

		ElemDrawer* drawer{nullptr};
		bool pressed = false;

	public:
		std::string name{"undefind"};
		Graphic::Color color{1.0f, 1.0f, 1.0f, 1.0f};
		mutable Graphic::Color tempColor{1.0f, 1.0f, 1.0f, 1.0f};

		mutable float maskOpacity = 1.0f;

		Geom::Vector2D margin_bottomLeft{};
		Geom::Vector2D margin_topRight{};

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
				bound.setSrcX(margin_bottomLeft.y);
				bound.setWidth(getHeight() - marginHeight());
			}

			return bound;
		}

		virtual bool layout_fillParent() {
			if(parent) {
				if(const Rect rect = parent->getFilledChildrenBound(this); rect != bound) {
					bound = rect;
					return true;
				}
			}

			return false;
		}

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

		virtual void layoutChildren() {
			for(const auto& child : children) {
				if(!child->ignoreLayout())child->layout();
			}
		}

		virtual bool ignoreLayout() const {
			return !visiable;
		}

		virtual void draw() const {
			drawBackground();
			drawChildren();
		}

		virtual void modifyAddedChildren(Elem* elem) {
			childrenCheck(elem);
			elem->parent = this;
			elem->changed();
			changed();
			elem->setRoot(root);
		}

		void setRoot(Root* const root) {
			this->root = root;

			std::for_each(std::execution::par_unseq, children.begin(), children.end(), [root](auto& elem) {
				elem->setRoot(root);
			});
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

		virtual void addChildren(std::unique_ptr<Elem>&& elem) {
			modifyAddedChildren(elem.get());
			children.push_back(std::forward<std::unique_ptr<Elem>>(elem));
		}

		virtual void addChildren(std::unique_ptr<Elem>&& elem, const size_t depth) {
			if(depth >= children.size()) {
				addChildren(std::forward<std::unique_ptr<Elem>>(elem));
			}else {
				modifyAddedChildren(elem.get());
				children.insert(children.begin() + depth, std::forward<std::unique_ptr<Elem>>(elem));
			}
		}

		virtual void addChildren(Elem* elem) {
			modifyAddedChildren(elem);
			children.emplace_back(elem);
		}

		virtual void addChildren(Elem* elem, const size_t depth) {
			if(depth >= children.size()) {
				addChildren(elem);
			}else {
				modifyAddedChildren(elem);
				children.insert(children.begin() + depth, std::unique_ptr<Elem>(elem));
			}
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

		virtual void drawChildren() const {
			for(const auto& elem : children) {
				elem->maskOpacity *= maskOpacity;
				elem->draw();
				elem->maskOpacity = 1.0f;
			}
		}

		[[nodiscard]] Elem* getParent() const {
			return parent;
		}

		void setDrawer(ElemDrawer* drawer) {
			this->drawer = drawer;
		}

		Elem* setParent(Elem* const parent) {
			Elem* former = parent;
			this->parent = parent;

			return former;
		}

		virtual Elem& prepareRemove() {
			if(parent == nullptr) {
				throw ext::NullPointerException{"This Elem: " + name + " Doesn't Have A Parent!"};
			}
			parent->toRemove.insert(this);

			return *this;
		}

		[[nodiscard]] bool endingRow() const {
			return endRow;
		}

		void setEndingRow(const bool end) {
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


		virtual std::vector<std::unique_ptr<Elem>>& getChildren() {
			return children;
		}

		virtual void calAbsolute(Elem* parent) {
			Geom::Vector2D vec{parent->absoluteSrc};
			vec.add(bound.getSrcX(), bound.getSrcY());
			if(vec == absoluteSrc)return;
			absoluteSrc.set(vec);
			calAbsoluteChildren();
		}

		virtual void calAbsoluteChildren() {
			std::for_each(std::execution::par_unseq, children.begin(), children.end(), [this](const std::unique_ptr<Elem>& elem) {
				elem->calAbsolute(this);
			});
		}

		Geom::Vector2D& getAbsSrc() {
			return absoluteSrc;
		}

		virtual int elemSerializationID() {
			return 0;
		}

		bool hasChildren() const {
			return !children.empty();
		}

		[[nodiscard]] Event::EventManager& getInputListener() {
			return inputListener;
		}

		[[nodiscard]] const Event::EventManager& getInputListener() const {
			return inputListener;
		}

		template <Concepts::Invokable<void(Elem*)> Func>
		void iterateAll(const Func& func) {
			func(this);

			for (const auto& child : getChildren()) {
				child->iterateAll(func);
			}
		}

		[[nodiscard]] bool hasChanged() const {
			return layoutChanged;
		}

		virtual void changed() const {
			layoutChanged = true;
			if(parent)parent->changed();
		}

		void toString(std::ostream& os, const int depth) const {
			//TODO tree print support
		}

		virtual void update(float delta){
			removePosted();
			updateChildren(delta);
		}

		virtual void removePosted() {
			if(toRemove.empty() || children.empty())return;
			const auto&& itr = std::remove_if(std::execution::par_unseq, children.begin(), children.end(), [this](const std::unique_ptr<Elem>& ptr) {
				return toRemove.contains(ptr.get());
			});
			if(itr == children.end())return;
			children.erase(itr);
			toRemove.clear();
		}

		virtual void updateChildren(const float delta) {
			std::for_each(std::execution::par_unseq, children.begin(), children.end(), [delta](const std::unique_ptr<Elem>& elem) {
				elem->update(delta);
			});
		}

		bool interactive() const {
			return touchbility == TouchbilityFlags::enabled && visiable;
		}

		virtual bool inbound_validToParent(const Geom::Vector2D& screenPos) const {
			return inbound(screenPos);
		}

		virtual bool inbound(const Geom::Vector2D& screenPos) const {
			if(touchbility == TouchbilityFlags::disabled)return false;
			if(parent != nullptr && !parent->inbound_validToParent(screenPos))return false;
			return screenPos.x > absoluteSrc.x && screenPos.y > absoluteSrc.y && screenPos.x < absoluteSrc.x + bound.getWidth() && screenPos.y < absoluteSrc.y + bound.getHeight();
		}

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

		virtual void childrenCheck(const Elem* ptr) {
#ifdef  DEBUG_LOCAL
			if(!ptr)throw ext::NullPointerException{"Empty Elem"};
#else
			return;
#endif
		}
	};
}
