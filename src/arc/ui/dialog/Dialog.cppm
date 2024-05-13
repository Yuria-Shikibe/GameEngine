//
// Created by Matrix on 2024/3/31.
//

export module UI.Dialog;

export import UI.Table;
import UI.SeperateDrawable;

import ext.Concepts;

import std;

export namespace UI{
	class Dialog : public SeperateDrawable{

		Dialog* parentDialog{nullptr};

		//TODO uses a set instead to support multi children dialogs?
		std::shared_ptr<Dialog> childDialog{nullptr};

		Dialog* toDestroy{nullptr};
		bool showing{false};


		//DialogShower
		bool coverParent{};
		bool fillScreen{true};
		Align::Spacing margin{8, 8, 8, 8};



		void initContent();

		void resizeContent();

	public:
		Dialog() = default;

		explicit Dialog(const bool fillScreen)
			: fillScreen{fillScreen}{
		}

		explicit Dialog(const bool fillScreen, Concepts::Invokable<void(UI::Table&)> auto&& func)
			: Dialog{fillScreen}{
			this->build(std::forward<decltype(func)>(func));
			content.changed(ChangeSignal::notifyAll);
		}

		explicit Dialog(Concepts::Invokable<void(UI::Table&)> auto&& func)
			: Dialog{true, std::forward<decltype(func)>(func)}{}

		explicit Dialog(Concepts::Invokable<void(UI::Dialog&)> auto&& func){
			func(*this);
			content.changed(ChangeSignal::notifyAll);
		}

		[[nodiscard]] Dialog* findFirstHiddenDialogNode(){
			if(!showing || !hasChild()){
				return this;
			}

			return childDialog->findFirstHiddenDialogNode();
		}

		[[nodiscard]] Dialog* findFirstShowingDialogNode(){
			if(showing || !hasChild()){
				return this;
			}

			return childDialog->findFirstShowingDialogNode();
		}

		[[nodiscard]] const Dialog* findFirstHiddenDialogNode() const{
			if(!showing || !hasChild()){
				return this;
			}

			return childDialog->findFirstHiddenDialogNode();
		}

		[[nodiscard]] const Dialog* findFirstShowingDialogNode() const{
			if(showing || !hasChild()){
				return this;
			}

			return childDialog->findFirstShowingDialogNode();
		}

		[[nodiscard]] bool isFillScreen() const{
			return fillScreen;
		}

		void setFillScreen(const bool fillScreen){
			this->fillScreen = fillScreen;

			resizeContent();
		}

		UI::Table content{};

		[[nodiscard]] bool hasParent() const{
			return parentDialog != nullptr;
		}

		[[nodiscard]] bool hasChild() const{
			return childDialog != nullptr;
		}

		void appendDialog(std::shared_ptr<Dialog>&& dialog){
			if(hasChild()){
				childDialog->destroy();
			}

			childDialog = std::move(dialog);
			childDialog->parentDialog = this;

			childDialog->initContent();
			childDialog->show();
		}

		void build(Concepts::Invokable<void(UI::Table&)> auto&& func){
			func(content);
		}

		[[nodiscard]] UI::Table& getContent(){ return content; }

		/**
		 * @brief Root dialog means there is no dialog to draw now
		 */
		[[nodiscard]] bool isRootDialog() const{
			return !hasParent();
		}

		/**
		 * @brief Deferred Destroy at next frame.
		 */
		void destroy(){
			if(hasParent()){
				parentDialog->toDestroy = this;
				hide(true);
				parentDialog = nullptr;
			}
		}

		void hide(const bool showParent){
			content.setVisible(false);
			showing = false;

			if(showParent && hasParent()){
				parentDialog->show();
			}
		}

		void show(){
			if(hasParent()){
				parentDialog->hide(false);
			}

			content.setVisible(true);
			showing = true;
		}

		void update(const float delta){
			if(toDestroy && toDestroy == childDialog.get()){
				childDialog.reset();
			}

			toDestroy = nullptr;

			if(isRootDialog()){
				if(hasChild())childDialog->update(delta);
			}else{
				if(hasChild()){
					childDialog->update(delta);
					if(!childDialog->coverParent){
						content.update(delta);
					}
				}else{
					content.update(delta);
				}
			}
		}

		void draw() const override{
			if(showing && !isRootDialog()){
				content.draw();
			}else if(hasChild()){
				childDialog->draw();
			}
		}

		void drawBase() const override{
			if(showing && !isRootDialog()){
				content.drawBase();
			}else if(hasChild()){
				childDialog->drawBase();
			}
		}

		void resize();

		// void
	};
}
