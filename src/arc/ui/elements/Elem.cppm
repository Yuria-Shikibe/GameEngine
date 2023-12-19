//
// Created by Matrix on 2023/12/3.
//

module;

export module UI.Elem;

export import UI.Flags;
import Event;
import Geom.Vector2D;
import Graphic.Color;
import Geom.Shape.Rect_Orthogonal;
import RuntimeException;

import UI.ElemDrawer;

import <algorithm>;
import <execution>;
import <functional>;
import <vector>;
import <unordered_set>;

using Rect = Geom::Shape::OrthoRectFloat;

export namespace UI {
class Root;

class Elem {
	public:
		virtual ~Elem() = default;

		[[nodiscard]] Elem() = default;

	protected:
		/**
		 * \brief The srcx, srcy is relative to its parent.
		 */
		Rect bound{};
		Elem* parent{nullptr};
		Root* root{nullptr};

		//TODO abstact this if possible
		std::vector<std::unique_ptr<Elem>> children{};
		std::vector<Elem*> toRemove{};

		std::unordered_set<Elem*> focusTarget{};

		Event::EventManager eventListener{};
		TouchbilityFlags touchbility = TouchbilityFlags::enabled;

		std::function<bool()> visibilityChecker{nullptr};

		mutable bool layoutChanged{false};
		bool endRow{false};
		bool visiable{true};

		Geom::Vector2D absoluteSrc{};

		ElemDrawer* drawer{UI::defDrawer.get()};

	public:
		std::string_view name{"undefind"};
		Graphic::Color color{1.0f, 1.0f, 1.0f, 1.0f};
		mutable float maskOpacity = 1.0f;

		[[nodiscard]] virtual bool isVisiable() const {
			return visiable;
		}

		virtual void layout() {

		}

		virtual void draw() const {
			drawBackground();
			drawChildren();
		}

		virtual void addChildren(Elem* elem) {
			nullPointerCheck(elem);
			children.emplace_back(elem);
			layoutChanged = true;
		}

		virtual void addChildren(Elem* elem, const size_t depth) {
			nullPointerCheck(elem);
			children.insert(children.begin() + std::clamp(depth, 0ull, children.size()), std::unique_ptr<Elem>(elem));
			layoutChanged = true;
		}

		virtual void drawBackground() const {
			drawer->drawBackground(absoluteSrc.x, absoluteSrc.y, getBound().getWidth(), getBound().getHeight(), color, maskOpacity);
		}

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
				throw ext::NullPointerException{"This Elem: " + static_cast<std::string>(name) + " Doesn't Have A Parent!"};
			}
			parent->toRemove.emplace_back(this);

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
			bound.setSrc(x, y);
			layoutChanged = true;
		}

		virtual void setWidth(const float w) {
			bound.setWidth(w);
			layoutChanged = true;
		}

		virtual void setHeight(const float h) {
			bound.setHeight(h);
			layoutChanged = true;
		}

		virtual void setSize(const float w, const float h) {
			bound.setSize(w, h);
			layoutChanged = true;
		}

		virtual void setSize(const float s) {
			bound.setSize(s, s);
			layoutChanged = true;
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
			absoluteSrc.set(parent->absoluteSrc).add(bound.getSrcX(), bound.getSrcY());
		}

		Geom::Vector2D& getAbsSrc() {
			return absoluteSrc;
		}

		virtual int elemSerializationID() {
			return 0;
		}

		[[nodiscard]] bool hasChanged() const {
			return layoutChanged;
		}

		void toString(std::ostream& os, const int depth) const {
			//TODO tree print support
		}

		virtual void update(float delta){
			updateChildren(delta);
		}

		virtual void updateChildren(const float delta) {
			while(!toRemove.empty()) {
				std::erase_if(children, [data = toRemove.back()](const std::unique_ptr<Elem>& ptr) {
					return ptr.get() == data;
				});
				toRemove.pop_back();
			}

			std::for_each(std::execution::par_unseq, children.begin(), children.end(), [delta](const std::unique_ptr<Elem>& elem) {
				elem->update(delta);
			});
		}

		bool isFocused() const;

		void setFocused(bool focus);

		static void nullPointerCheck(const void* ptr) {
#ifdef  DEBUG_LOCAL
			if(!ptr)throw ext::NullPointerException{"Empty Elem"};
#else
			return;
#endif
		}
	};
}
