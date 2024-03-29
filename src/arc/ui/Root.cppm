module;

export module UI.Root;

import std;
import Concepts;
import Container.Pool;

import Geom.Matrix3D;

export import UI.Flags;

import Ctrl.Constants;
import Graphic.Resizeable;
import OS.ApplicationListener;
import OS.InputListener;

import UI.Elem;
import UI.Table;

//TODO layout update inform system: current layout process is totally mess!
export namespace UI{
	class Root : public Graphic::Resizeable<unsigned int>, public OS::ApplicationListener,
	             public OS::InputListener {
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
		float width{ 0 };
		float height{ 0 };

		float marginX = 8.0f;
		float marginY = 8.0f;

		bool allHidden = false;

		[[nodiscard]] Root();

		Geom::Vec2 cursorPos{};
		Geom::Vec2 cursorPressedBeginPos{};
		Geom::Vec2 cursorVel{};
		Geom::Vec2 mouseScroll{};

		//TODO use bitmap or array???
		std::bitset<Ctrl::MOUSE_BUTTON_COUNT> pressedMouseButtons{};

		Elem* currentInputFocused{ nullptr };
		Elem* currentScrollFocused{ nullptr };
		const Elem* currentCursorFocus{ nullptr };
		// // //Focus
		// //
		// // //Input Listeners
		std::unique_ptr<Table> root{ nullptr };

		[[nodiscard]] bool mouseFocusFree() const {
			return currentCursorFocus == nullptr;
		}

		[[nodiscard]] bool focusScroll() const {
			return currentScrollFocused != nullptr;
		}

		[[nodiscard]] bool focusKeyInput() const {
			return currentInputFocused != nullptr;
		}

		[[nodiscard]] bool cursorCaptured() const {
			return currentCursorFocus != nullptr;
		}

		void iterateAll_DFS(Elem* current, bool& shouldStop, Concepts::Invokable<bool(Elem*)> auto&& func){
			if (shouldStop)return;

			if (!func(current) || shouldStop)return;

			if(!current->hasChildren())return;

			for (auto& child : *current->getChildren()) {
				this->iterateAll_DFS(child.get(), shouldStop, std::forward<decltype(func)>(func));
			}
		}

		void update(float delta) override;

		[[nodiscard]] Geom::Matrix3D& getPorj() {
			return projection;
		}

		//TODO shit named fucntion and logic!
		void determinShiftFocus(const Elem* newFocus);

		void resize(unsigned w, unsigned h) override;

		[[nodiscard]] float getMarginX() const {return marginX;}

		[[nodiscard]] float getMarginY() const {return marginY;}

		[[nodiscard]] float getWidth() const {return width;}

		[[nodiscard]] float getHeight() const {return height;}

		virtual void render() const;

		[[nodiscard]] Geom::Vec2 getCursorDst() const{
			return cursorPos - cursorPressedBeginPos;
		}

		//TODO mode support
		void onDoubleClick(int id, int mode = 0) const;

		void onPress(const int id, int mode = 0);

		void onRelease(const int id, int mode = 0);

		void onScroll() const;

		void disable();

		void enable();

		[[nodiscard]] bool onDrag(const int id, int mode = 0) const;

		void onDragUpdate() const;

	protected:
		void inform(int keyCode, int action, int mods) override {
		}

		void setEnter(const Elem* elem);
	};
}
