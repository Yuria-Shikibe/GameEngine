module;

export module UI.Group;

export import UI.Elem;
export import UI.Flags;
import ext.RuntimeException;

import ext.Concepts;

import std;

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
				if(!child->isIgnoreLayout())child->layout();
			}
		}

		void postRemove(Elem* elem) {
			toRemove.insert(elem);
		}

		void setRoot(Root* root) noexcept override{
			Elem::setRoot(root);

			for(const auto& elem : children){
				elem->setRoot(root);
			}
		}

		virtual Elem* addChildren(std::unique_ptr<Elem>&& elem) {
			modifyAddedChildren(elem.get());
			children.push_back(std::forward<std::unique_ptr<Elem>>(elem));
			return children.back().get();
		}

		virtual Elem* addChildren(std::unique_ptr<Elem>&& elem, const int depth) {
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

		virtual void addChildren(Elem* elem, const std::size_t depth) {
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

		void calAbsoluteChildren() noexcept override{
			std::for_each(std::execution::unseq, children.begin(), children.end(), [this](const std::unique_ptr<Elem>& elem) {
				elem->calAbsoluteSrc(this);
			});
		}

		const std::vector<std::unique_ptr<Elem>>& getChildren() const noexcept override {
			return children;
		}

		bool hasChildren() const noexcept override {
			return !children.empty();
		}

		void layout() override {
			layoutChildren();

			Elem::layout();
		}

		void iterateAll(Concepts::Invokable<void(Elem*)> auto&& func) {
			func(this);

			for (const auto& child : getChildren()) {
				auto* group = dynamic_cast<Group*>(child.get());
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
			for (auto& child : children){
				if(!child->isSleep())child->update(delta);
			}
		}

		virtual void clearChildrenInstantly(){
			children.clear();
		}

		void clearChildrenSafely() const{
			for(auto& element : children){
				element->callRemove();
				element->setSleep(true);
				element->setVisible(false);
			}
		}

		void update(const Core::Tick delta) override {
			removePosted();

			Elem::update(delta);

			updateChildren(delta);
		}

		void setDisabled(const bool disabled) noexcept override{
			if(this->disabled != disabled){
				this->disabled = disabled;
				for (const auto& child : children){
					child->setDisabled(disabled);
				}
			}
		}

		void drawBase() const override{
			Elem::drawBase();

			if(visiable)for(const auto& elem : children) {
				elem->drawBase();
			}
		}

		void drawContent() const override {
			if(visiable)drawChildren();
		}

		void postChanged() noexcept override{
			Elem::postChanged();

			if(lastSignal & ChangeSignal::notifyChildrenOnly){
				for(const auto& element : children){
					element->changed(lastSignal, ChangeSignal::notifyParentOnly);
					element->postChanged();
				}
			}
		}

		bool onEsc() override{
			bool rst = true;
			for(const auto& child : children){
				if(child->onEsc()){
					continue;
				}

				rst = false;
				break;
			}

			return rst;
		}

		void childrenCheck(const Elem* ptr) override{
#ifdef  _DEBUG
			if(!ptr)throw ext::NullPointerException{"Empty Elem"};
#else
			return;
#endif
		}
	};
}
