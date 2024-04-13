module;

export module UI.Group;

export import UI.Elem;
export import UI.Flags;
import RuntimeException;

import Concepts;

import std;

export namespace UI {
	class Group : public Widget{
	protected:
		//TODO abstact this if possible
		std::vector<std::unique_ptr<Widget>> children{};
		std::unordered_set<Widget*> toRemove{};

	public:
		[[nodiscard]] Group() {
			touchbility = TouchbilityFlags::childrenOnly;
		}

		virtual void layoutChildren() {
			for(const auto& child : children) {
				if(!child->isIgnoreLayout())child->layout();
			}
		}

		void postRemove(Widget* elem) {
			toRemove.insert(elem);
		}

		void setRoot(Root* root) override{
			Widget::setRoot(root);

			for(const auto& elem : children){
				elem->setRoot(root);
			}
		}

		virtual Widget* addChildren(std::unique_ptr<Widget>&& elem) {
			modifyAddedChildren(elem.get());
			children.push_back(std::forward<std::unique_ptr<Widget>>(elem));
			return children.back().get();
		}

		virtual Widget* addChildren(std::unique_ptr<Widget>&& elem, const int depth) {
			if(depth >= children.size()) {
				addChildren(std::forward<std::unique_ptr<Widget>>(elem));
				return children.back().get();
			}else{
				modifyAddedChildren(elem.get());
				return children.insert(children.begin() + depth, std::forward<std::unique_ptr<Widget>>(elem))->get();
			}
		}

		virtual void addChildren(Widget* elem) {
			modifyAddedChildren(elem);
			children.emplace_back(elem);
		}

		virtual void addChildren(Widget* elem, const int depth) {
			if(depth >= children.size()) {
				addChildren(elem);
			}else {
				modifyAddedChildren(elem);
				children.insert(children.begin() + depth, std::unique_ptr<Widget>(elem));
			}
		}

		virtual void drawChildren() const {
			for(const auto& elem : children) {
				elem->draw();
			}
		}

		virtual void modifyAddedChildren(Widget* elem);

		void calAbsoluteChildren() override{
			std::for_each(std::execution::unseq, children.begin(), children.end(), [this](const std::unique_ptr<Widget>& elem) {
				elem->calAbsoluteSrc(this);
			});
		}

		const std::vector<std::unique_ptr<Widget>>* getChildren() const override {
			return &children;
		}

		bool hasChildren() const override {
			return !children.empty();
		}

		void layout() override {
			layoutChildren();

			Widget::layout();
		}

		void iterateAll(Concepts::Invokable<void(Widget*)> auto&& func) {
			func(this);

			for (const auto& child : getChildren()) {
				auto* group = dynamic_cast<Group*>(child.get());
				if(!group)return;
				group->iterateAll(std::forward<decltype(func)>(func));
			}
		}

		virtual void removePosted() {
			if(toRemove.empty() || children.empty())return;
			std::erase_if(children, [this](const std::unique_ptr<Widget>& ptr) {
				return toRemove.contains(ptr.get());
			});
			toRemove.clear();
		}

		virtual void updateChildren(const float delta) {
			for (auto& child : children){
				child->update(delta);
			}
		}

		void update(const float delta) override {
			removePosted();

			Widget::update(delta);

			updateChildren(delta);
		}

		void setDisabled(const bool disabled) override{
			if(this->disabled != disabled){
				this->disabled = disabled;
				for (const auto& child : children){
					child->setDisabled(disabled);
				}
			}
		}

		void drawBase() const override{
			Widget::drawBase();

			if(visiable)for(const auto& elem : children) {
				elem->drawBase();
			}
		}

		void drawContent() const override {
			if(visiable)drawChildren();
		}

		void postChanged() override{
			Widget::postChanged();

			if(lastSignal & ChangeSignal::notifyChildrenOnly){
				for(auto& element : children){
					element->changed(lastSignal, ChangeSignal::notifyParentOnly);
					element->postChanged();
				}
			}
		}

		void childrenCheck(const Widget* ptr) override{
#ifdef  _DEBUG
			if(!ptr)throw ext::NullPointerException{"Empty Elem"};
#else
			return;
#endif
		}
	};
}
