module;

export module UI.Root;

import ext.Concepts;
import ext.Container.ObjectPool;

import Geom.Matrix3D;

export import UI.Flags;
export import UI.TooltipManager;
import UI.SeperateDrawable;

import OS.Ctrl.Bind.Constants;
import Graphic.Resizeable;
import OS.TextInputListener;
import OS.InputListener;

export import UI.Dialog;
export import UI.Elem;
export import UI.Scene;

import OS.Ctrl.Bind;
import ext.Heterogeneous;
import Assets.Bundle;

import std;


//TODO layout update inform system: current layout process is totally mess!
export namespace UI{
	class Root : public Graphic::ResizeableInt,
	             public OS::InputListener{
	protected:
		mutable MouseActionPress pressAction{};
		mutable MouseActionRelease releaseAction{};
		mutable MouseActionDoubleClick doubleClickAction{};
		mutable MouseActionDrag dragAction{};

		mutable MouseActionScroll scrollAction{};

		mutable CurosrInbound inboundAction{};
		mutable CurosrExbound exboundAction{};

		Geom::Matrix3D projection{};
		std::unordered_map<const Elem*, OS::InputBindGroup*> customeInputBinds{};

		void setRootOf(Elem* widget);

	public:
		Assets::Bundle uiBasicBundle{};
		TooltipManager tooltipManager{};
		Dialog rootDialog{};

		float width{0};
		float height{0};

		bool isHidden = false;

		bool tempTooltipBanned = false;

		[[nodiscard]] Root();

		~Root() override;

		Geom::Vec2 cursorPos{};
		Geom::Vec2 cursorPressedBeginPos{};
		Geom::Vec2 cursorVel{};
		Geom::Vec2 mouseScroll{};

		//TODO use bitmap or array??? this is a byte long seroulsy
		std::array<int, Ctrl::Mouse::Count> pressedMouseButtons{};

		//TODO is this necessary?
		Elem* currentInputFocused{nullptr};
		Elem* currentScrollFocused{nullptr};
		Elem* currentCursorFocus{nullptr};

		//TODO directly use InputArea instead?
		//redundant virtual
		OS::TextInputListener* textInputListener{nullptr};
		OS::InputListener* inputListener{nullptr};

		Scene* currentScene{nullptr};

		OS::InputBindGroup uiInput{};

		CursorType currentCursorType = CursorType::regular;

		float cursorStrandedTime{0.0f};
		float cursorInBoundTime{0.0f};

		void loadBinds(const Elem* elem, OS::InputBindGroup& binds);
		void unloadBinds(const Elem* elem);

		[[nodiscard]] const ext::StringHashMap<std::unique_ptr<Scene>>& getScenes() const{ return scenes; }

		[[nodiscard]] constexpr bool noMousePress() const noexcept{
			return std::ranges::all_of(pressedMouseButtons, Ctrl::Mode::isDisabled);
		}

		template <Concepts::Derived<Scene> T, typename  ...Args>
		void registerScene(const std::string_view name, Args&& ...args){
			std::unique_ptr<Scene> ptr = std::make_unique<T>(std::forward<Args>(args) ...);
			setRootOf(ptr.get());
			ptr->setSize(width, height);
			ptr->build();

			scenes.insert_or_assign(name, std::move(ptr));
		}

		void registerScene(std::unique_ptr<Scene>&& scene){
			auto name = std::string(scene->getSceneName());

			setRootOf(scene.get());
			scene->setSize(width, height);
			scene->build();

			scenes.insert_or_assign(std::move(name), std::move(scene));
		}

		void eraseScene(const std::string_view name){
			scenes.erase(name);
		}

		UI::Dialog* handleDialog(std::shared_ptr<Dialog>&& dialog){
			releaseFocus();

			tooltipManager.dropCurrentAt(nullptr, dialog->isFillScreen());
			setRootOf(&dialog->content);
			const auto ptr = dialog.get();
			ptr->build();
			rootDialog.findFirstShowingDialogNode()->appendDialog(std::move(dialog));

			return ptr;
		}

		template <Concepts::Derived<UI::Dialog> Dy>
		void showDialog(const bool fillScreen = true){
			this->handleDialog(std::make_shared<Dy>(fillScreen, this));
		}

		template <Concepts::Invokable<void(UI::Table&)> Builder>
		void showDialog(const bool fillScreen, Builder&& func){
			this->handleDialog(std::make_shared<Dialog>(fillScreen, this, std::forward<Builder>(func)));
		}

		template <Concepts::Invokable<void(UI::Table&)> Builder>
		void showDialog(Builder&& func){
			this->showDialog<Builder>(true, std::forward<Builder>(func));
		}

		template <Concepts::Invokable<void(UI::Dialog&)> InitFunc>
		void showDialog(const bool fillScreen, InitFunc&& func){
			this->handleDialog(std::make_shared<Dialog>(fillScreen, this, std::forward<InitFunc>(func)));
		}

		void switchScene(const std::string_view sceneName){
			if(const auto nextItr = scenes.find(sceneName); nextItr != scenes.end()){
				currentScene->setVisible(false);
				currentScene = nextItr->second.get();
				if(currentScene){
					currentScene->setSize(width, height);
					currentScene->setVisible(true);
				}
			}else{
				//TODO throw maybe
			}
		}

		[[nodiscard]] bool mouseFocusFree() const{
			return currentCursorFocus == nullptr;
		}

		[[nodiscard]] bool focusScroll() const{
			return currentScrollFocused != nullptr;
		}

		[[nodiscard]] bool focusKeyInput() const{
			return currentInputFocused != nullptr;
		}

		[[nodiscard]] bool cursorCaptured() const{
			return currentCursorFocus != nullptr;
		}

		void releaseFocus(){
			setEnter(nullptr);
			currentCursorFocus = nullptr;
			currentScrollFocused = nullptr;
			currentInputFocused = nullptr;
			textInputListener = nullptr;
		}

		void iterateAll_DFS(Elem* current, Concepts::Invokable<bool(Elem*)> auto&& func){
			if(!func(current)) return;

			if(!current->hasChildren()) return;

			for(auto& child : current->getChildrenView()){
				this->iterateAll_DFS(child.get(), std::forward<decltype(func)>(func));
			}
		}


		void updateCurrentFocus(){
			Elem* last = nullptr;

			Elem* toIterate = nullptr;


			if(!tooltipManager.focusEmpty()){
				toIterate = tooltipManager.findFocus();
			}

			if(!toIterate){
				if(auto* lastDialog = rootDialog.findFirstShowingDialogNode(); lastDialog != &rootDialog){
					toIterate = &lastDialog->content;
				}else{
					toIterate = currentScene;
				}
			}

			if(toIterate){
				iterateAll_DFS(toIterate, [this, &last](Elem* elem) mutable{
					if(elem->isInteractable() && elem->isInbound(cursorPos)){
						last = elem;
					}

					return !elem->touchDisabled();
				});
			}

			determinShiftFocus(last);
		}

		void drawCursor() const;

		void update(const Core::Tick delta);

		[[nodiscard]] Geom::Matrix3D& getPorj(){
			return projection;
		}

		//TODO shit named fucntion and logic!
		void determinShiftFocus(Elem* newFocus);

		void setTextFocus(OS::TextInputListener* listener){
			this->textInputListener = listener;
		}

		[[nodiscard]] bool hasTextFocus() const noexcept{
			return this->textInputListener != nullptr;
		}

		void resize(int w, int h) override;

		[[nodiscard]] float getWidth() const{ return width; }

		[[nodiscard]] float getHeight() const{ return height; }

		void render() const;

		void renderBase() const;

		void renderBase_HoverTable() const;

		void render_HoverTable() const;

		bool hasDialog() const{
			return rootDialog.findFirstShowingDialogNode() != &rootDialog;
		}

		[[nodiscard]] Geom::Vec2 getCursorDst() const{
			return cursorPos - cursorPressedBeginPos;
		}

		/**
		 * @return true if Quit Happens
		 */
		bool quitTopDialog(){
			if(auto* last = rootDialog.findFirstShowingDialogNode(); last != &rootDialog){
				if(last->content.onEsc()){
					if(last->tryEsc())last->destroy();
				}

				return true;
			}

			return false;
		}

		/**
		 * @return true if there is nothing can do
		 */
		bool onEsc(){
			if(this->textInputListener){
				textInputListener->informEscape(0, 0);
				this->setTextFocus(nullptr);
				return false;
			}

			if(tooltipManager.dropBack()){
				tempTooltipBanned = true;
				return false;
			}

			return !quitTopDialog();
		}

		//TODO mode support
		void onDoubleClick(int id, int mode);

		void onPress(int id, int mode);

		void onRelease(int id, int mode);

		void onScroll() const;

		[[nodiscard]] bool onDrag(const int id) const noexcept(!DEBUG_CHECK){
#if DEBUG_CHECK
			if(id > Ctrl::Mouse::Count)throw ext::IllegalArguments{std::format("Illegal Mouse Button: {}", id)};
#endif

			return !Ctrl::Mode::isDisabled(pressedMouseButtons[id]) && currentCursorFocus != nullptr;
		}

		void onDragUpdate() const;

		bool keyDown(int code, int action, int mode) const;

		void textInputInform(const unsigned int code, const int mods) const{
			if(textInputListener){
				textInputListener->informTextInput(code, mods);
			}
		}

		void registerCtrl();

		constexpr void show() noexcept{
			isHidden = false;
		}

		constexpr void hide() noexcept{
			isHidden = true;
		}

		void handleSound(SoundSource sound);

	protected:
		void inform(const int keyCode, const int action, const int mods) override{
			if(inputListener){
				inputListener->inform(keyCode, action, mods);
			}
		}

		void setEnter(Elem* elem, bool quiet = false);

		//Make sure it is the first to destruct!
		ext::StringHashMap<std::unique_ptr<Scene>> scenes{};
	};
}
