module;

export module UI.Root;

import std;
import ext.Concepts;
import ext.Container.ObjectPool;

import Geom.Matrix3D;

export import UI.Flags;
export import UI.TooltipManager;
export import UI.Dialog;
import UI.SeperateDrawable;

import OS.Ctrl.Bind.Constants;
import Graphic.Resizeable;
import OS.ApplicationListener;
import OS.TextInputListener;
import OS.InputListener;

import UI.Widget;
import UI.Scene;
import Core.Input;
import ext.Heterogeneous;

//TODO layout update inform system: current layout process is totally mess!
export namespace UI{
	class Root : public Graphic::ResizeableInt, public OS::ApplicationListener,
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

	public:
		TooltipManager tooltipManager{};
		Dialog rootDialog{};

		float width{0};
		float height{0};

		bool isHidden = false;

		[[nodiscard]] Root();

		~Root() override;

		Geom::Vec2 cursorPos{};
		Geom::Vec2 cursorPressedBeginPos{};
		Geom::Vec2 cursorVel{};
		Geom::Vec2 mouseScroll{};

		//TODO use bitmap or array??? this is a byte long seroulsy
		std::array<int, Ctrl::Mouse::Count> pressedMouseButtons{};

		//TODO is this necessary?
		Widget* currentInputFocused{nullptr};
		Widget* currentScrollFocused{nullptr};
		Widget* currentCursorFocus{nullptr};

		//TODO directly use InputArea instead?
		//redundant virtual
		OS::TextInputListener* textInputListener{nullptr};

		ext::StringMap<std::unique_ptr<Scene>> scenes{};

		Scene* currentScene{nullptr};

		Core::InputBindGroup uiInput{};

		CursorType currentCursorType = CursorType::regular;

		float cursorStrandedTime{0.0f};
		float cursorInBoundTime{0.0f};

		[[nodiscard]] constexpr bool noMousePress() const noexcept{
			return std::ranges::all_of(pressedMouseButtons, Ctrl::Mode::isDisabled);
		}

		void showDialog(std::shared_ptr<Dialog>&& dialog){
			releaseFocus();
			tooltipManager.dropCurrentAt(nullptr, dialog->isFillScreen());
			rootDialog.findFirstShowingDialogNode()->appendDialog(std::move(dialog));
		}

		template <Concepts::Invokable<void(UI::Table&)> Builder>
		void showDialog(const bool fillScreen, Builder&& func){
			this->showDialog(std::make_shared<Dialog>(fillScreen, std::forward<Builder>(func)));
		}

		template <Concepts::Invokable<void(UI::Table&)> Builder>
		void showDialog(Builder&& func){
			this->showDialog<Builder>(true, std::forward<Builder>(func));
		}

		template <Concepts::Invokable<void(UI::Dialog&)> InitFunc>
		void showDialog(InitFunc&& func){
			this->showDialog(std::make_shared<Dialog>(std::forward<InitFunc>(func)));
		}

		void switchScene(const std::string_view sceneName){
			if(const auto nextItr = scenes.find(sceneName); nextItr != scenes.end()){
				currentScene->setVisible(false);
				currentScene = nextItr->second.get();
				if(currentScene)currentScene->setSize(width, height);
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

		void iterateAll_DFS(Widget* current, Concepts::Invokable<bool(Widget*)> auto&& func){
			if(!func(current)) return;

			if(!current->hasChildren()) return;

			for(auto& child : *current->getChildren()){
				this->iterateAll_DFS(child.get(), std::forward<decltype(func)>(func));
			}
		}


		void updateCurrentFocus(){
			Widget* last = nullptr;

			Widget* toIterate = nullptr;


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
				iterateAll_DFS(toIterate, [this, &last](Widget* elem) mutable{
					if(elem->isInteractable() && elem->isInbound(cursorPos)){
						last = elem;
					}

					return !elem->touchDisabled();
				});
			}

			determinShiftFocus(last);
		}

		void drawCursor() const;

		void update(float delta) override;

		[[nodiscard]] Geom::Matrix3D& getPorj(){
			return projection;
		}

		//TODO shit named fucntion and logic!
		void determinShiftFocus(Widget* newFocus);

		void setTextFocus(OS::TextInputListener* listener){
			this->textInputListener = listener;
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
		 * @return true if there is nothing can do
		 */
		bool onEsc(){
			if(this->textInputListener){
				this->setTextFocus(nullptr);
				return false;
			}

			if(auto* last = rootDialog.findFirstShowingDialogNode(); last != &rootDialog){
				if(last->content.onEsc()){
					last->destroy();
				}

				return false;
			}

			return true;
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

	protected:
		void inform(int keyCode, int action, int mods) override{}

		void setEnter(Widget* elem, bool quiet = false);

	};
}
