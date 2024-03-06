module;

export module UI.Group;

export import UI.Elem;

import Concepts;

import <execution>;
import <memory>;
import <vector>;
import <unordered_set>;

export namespace UI {
	class Group : public Elem{
	protected:
		//TODO abstact this if possible
		std::vector<std::unique_ptr<Elem>> children{};
		std::unordered_set<Elem*> toRemove{};

	public:
		[[nodiscard]] Group() {
			touchbility = TouchbilityFlags::childrenOnly;
		}

		virtual void layoutChildren() {
			for(const auto& child : children) {
				if(!child->ignoreLayout())child->layout();
			}
		}

		void postRemove(Elem* elem) {
			toRemove.insert(elem);
		}

		void setRoot(Root* root) override;

		virtual Elem* addChildren(std::unique_ptr<Elem>&& elem) {
			modifyAddedChildren(elem.get());
			children.push_back(std::forward<std::unique_ptr<Elem>>(elem));
			return children.back().get();
		}

		virtual Elem* addChildren(std::unique_ptr<Elem>&& elem, const size_t depth) {
			if(depth >= children.size()) {
				addChildren(std::forward<std::unique_ptr<Elem>>(elem));
				return children.back().get();
			}else{
				modifyAddedChildren(elem.get());
				return children.insert(children.begin() + depth, std::forward<std::unique_ptr<Elem>>(elem))->get();
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

		virtual void drawChildren() const {
			for(const auto& elem : children) {
				elem->draw();
			}
		}

		virtual void modifyAddedChildren(Elem* elem);

		virtual void calAbsoluteChildren() {
			std::for_each(std::execution::par_unseq, children.begin(), children.end(), [this](const std::unique_ptr<Elem>& elem) {
				elem->calAbsolute(this);
			});
		}

		void calAbsolute(Elem* parent) override {
			Elem::calAbsolute(parent);

			calAbsoluteChildren();
		}

		std::vector<std::unique_ptr<Elem>>* getChildren() override {
			return &children;
		}

		bool hasChildren() const override {
			return !children.empty();
		}

		void layout() override {
			layoutChildren();

			Elem::layout();
		}

		void iterateAll(Concepts::Invokable<void(Elem*)> auto&& func) {
			func(this);

			for (const auto& child : getChildren()) {
				auto group = dynamic_cast<Group*>(child.get());
				if(!group)return;
				group->iterateAll(std::forward<decltype(func)>(func));
			}
		}

		virtual void removePosted() {
			if(toRemove.empty() || children.empty())return;
			std::erase_if(children, [this](const std::unique_ptr<Elem>& ptr) {
				return toRemove.contains(ptr.get());
			});
			toRemove.clear();
		}

		virtual void updateChildren(const float delta) {
			//TODO worth it a 'par' ?
			std::ranges::for_each(children, [delta](const std::unique_ptr<Elem>& elem) {
				elem->update(delta);
			});
		}

		void update(const float delta) override {
			removePosted();
			updateChildren(delta);
		}

		void drawContent() const override {
			drawChildren();
		}
	};
}
