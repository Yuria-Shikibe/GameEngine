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
import <vector>;
import <unordered_set>;

using Rect = Geom::Shape::OrthoRectFloat;

export namespace UI {
	class Elem {
	public:
		virtual ~Elem() = default;

	protected:
		Rect bound{};
		Elem* parent{nullptr};
		std::vector<Elem*> children{};
		std::unordered_set<Elem*> focusTarget{};
		Event::EventManager eventListener{};

		std::vector<Elem*> toRemove{};

	public:
		std::string name{};
		Graphic::Color color{};

		virtual void layout() = 0;

		virtual void draw() const = 0;

		[[nodiscard]] virtual Elem* getParent() const {
			return parent;
		}

		virtual Elem* setParent(Elem* const parent) {
			Elem* former = parent;
			this->parent = parent;

			return former;
		}

		virtual Elem& prepareRemove() {
			if(parent == nullptr) {
				throw ext::NullPointerException{"This Elem: " + name + " Doesn't Have A Parent!"};
			}
			parent->toRemove.push_back(this);

			return *this;
		}

		[[nodiscard]] virtual const std::unordered_set<Elem*>& getFocus() const {
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
		}

		virtual void setWidth(const float w) {
			bound.setWidth(w);
		}

		virtual void setHeight(const float h) {
			bound.setHeight(h);
		}

		virtual void setSize(const float w, const float h) {
			bound.setSize(w, h);
		}

		virtual void setSize(const float s) {
			bound.setSize(s, s);
		}

		[[nodiscard]] const Rect& getBound() const {
			return bound;
		}

		virtual std::vector<Elem*>& getChildren() {
			return children;
		}

		virtual int elemID() {
			return 0;
		}

		virtual bool deletable() {
			return parent == nullptr;
		}

		void toString(std::ostream& os, const int depth) const {
			//...
		}

		virtual void update(const float delta) {
			while(!toRemove.empty()) {
				std::erase(children, toRemove.back());
				toRemove.pop_back();
			}

			std::for_each(std::execution::par_unseq, children.begin(), children.end(), [delta](Elem* elem) {
				elem->update(delta);
			});
		}


	};
}
