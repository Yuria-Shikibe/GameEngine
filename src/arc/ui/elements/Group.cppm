module;

export module UI.Group;

export import UI.Elem;

import <execution>;
import <memory>;
import <vector>;
import <set>;

export namespace UI {
	class Group : public Elem{
	protected:
		//TODO abstact this if possible
		std::vector<std::unique_ptr<Elem>> children{};
		std::set<Elem*> toRemove{};

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

		virtual void drawChildren() const {
			for(const auto& elem : children) {
				elem->maskOpacity *= maskOpacity;
				elem->draw();
				elem->maskOpacity = 1.0f;
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

		template <Concepts::Invokable<void(Elem*)> Func>
		void iterateAll(const Func& func) {
			func(this);

			for (const auto& child : getChildren()) {
				auto group = dynamic_cast<Group*>(child.get());
				if(!group)return;
				group->iterateAll(func);
			}
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

		void update(const float delta) override {
			removePosted();
			updateChildren(delta);
		}

		void draw() const override {
			Elem::draw();

			drawChildren();
		}
	};
}
