//
// Created by Matrix on 2023/12/3.
//

module;

export module UI.Elem;

export import UI.Flags;
import Event;
import Graphic.Color;
import Geom.Shape.Rect_Orthogonal;
import RuntimeException;

import <algorithm>;
import <execution>;
import <functional>;
import <vector>;
import <unordered_set>;

using Rect = Geom::Shape::OrthoRectFloat;

export namespace UI {
	class Elem {
	public:
		virtual ~Elem() = default;

	protected:
		/**
		 * \brief The srcx, srcy is relative to its parent.
		 */
		Rect bound{};
		Elem* parent{nullptr};

		//TODO abstact this if possible
		std::vector<Elem*> children{};
		std::vector<Elem*> toRemove{};

		std::unordered_set<Elem*> focusTarget{};

		Event::EventManager eventListener{};
		TouchbilityFlags touchbility = TouchbilityFlags::enabled;

		std::function<bool()> visibilityChecker{nullptr};

		bool layoutChanged{false};
		bool endRow{false};
		bool visiable{true};

	public:
		std::string_view name{"undefind elem"};
		Graphic::Color color{1.0f, 1.0f, 1.0f, 1.0f};
		mutable float maskOpacity = 1.0f;

		[[nodiscard]] virtual bool isVisiable() const {
			return visiable;
		}

		virtual void layout() = 0;

		virtual void draw() const = 0;

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

		Elem* setParent(Elem* const parent) {
			Elem* former = parent;
			this->parent = parent;

			return former;
		}

		virtual Elem& prepareRemove() {
			if(parent == nullptr) {
				throw ext::NullPointerException{"This Elem: " + static_cast<std::string>(name) + " Doesn't Have A Parent!"};
			}
			parent->toRemove.push_back(this);

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

		virtual std::vector<Elem*>& getChildren() {
			return children;
		}

		virtual int elemSerializationID() {
			return 0;
		}

		virtual bool deletable() {
			return parent == nullptr;
		}

		[[nodiscard]] bool hasChanged() const {
			return layoutChanged;
		}

		void toString(std::ostream& os, const int depth) const {
			//TODO tree print support
		}

		virtual void update(float delta) = 0;

		virtual void updateChildren(const float delta) {
			while(!toRemove.empty()) {
				std::erase(children, toRemove.back());
				toRemove.pop_back();
			}

			std::for_each(std::execution::par_unseq, children.begin(), children.end(), [delta](Elem* elem) {
				elem->updateChildren(delta);
			});
		}
	};
}
