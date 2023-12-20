module;

export module UI.Root;

import <memory>;
import <vector>;
import <algorithm>;
import <array>;
import Concepts;
import Container.Pool;

import Geom.Matrix3D;

export import UI.Flags;
export import UI.Elem;
export import UI.Table;

import Ctrl.Constants;
import Graphic.Resizeable;
import OS.ApplicationListener;
import OS.InputListener;

//TODO layout update inform system: current layout process is totally mess!
export namespace UI{
class Root : public Graphic::Resizeable<unsigned int>, public OS::ApplicationListener, public OS::InputListener{
	protected:
	mutable MouseActionPress pressAction{};
	mutable MouseActionRelease releaseAction{};
	mutable MouseActionDoubleClick doubleClickAction{};
	mutable MouseActionDrug drugAction{};

	mutable CurosrInbound inboundAction{};
	mutable CurosrExbound exboundAction{};

	Geom::Matrix3D projection{};
	public:
		float width{0};
		float height{0};

		float marginX = 8.0f;
		float marginY = 8.0f;

		~Root() override = default;

		[[nodiscard]] Root() : root(std::make_unique<Table>()) { // NOLINT(*-use-equals-default)
			root->setSrc(0.0f, 0.0f);
			root->getAbsSrc().setZero();
			root->relativeLayoutFormat = false;
			root->setTouchbility(UI::TouchbilityFlags::childrenOnly);
		}

		Geom::Vector2D cursorPos{};
		Geom::Vector2D cursorVel{};

		std::array<bool, Ctrl::MOUSE_BUTTON_COUNT> pressedMouseButtons{};

		Elem* currentInputFocused{nullptr};

		const Elem* currentCursorFocus{nullptr};
		// // //Focus
		// //
		// // //Input Listeners
		std::unique_ptr<Table> root{nullptr};


		template <Concepts::Invokable<bool(Elem*)> Func>
		void iterateAll_DFS(Elem* current, bool& shouldStop, Func&& func) {
			if (shouldStop) {return;}



			if (!func(current) || shouldStop) {return;}

			for (auto& child : current->getChildren()) {
				iterateAll_DFS(child.get(), shouldStop, func);
			}
		}
		//
		// //Renderers
		//
		void update(const float delta) override {
			root->update(delta);

			bool foundInbounded = false;

			iterateAll_DFS(root.get(), foundInbounded, [&foundInbounded, this](const Elem* elem) mutable {
				foundInbounded = elem->interactive() && elem->inbound(cursorPos);
				if(foundInbounded) {
					setEnter(elem);
				}

				return !elem->touchDisabled();
			});

			if(!foundInbounded) {
				if(currentCursorFocus != nullptr) {
					if(currentCursorFocus->quitMouseFocusAtOutbound()) {
						setEnter(nullptr);
					}else if(std::ranges::none_of(pressedMouseButtons, std::identity{})){
						setEnter(nullptr);
					}
				}
			}

			onDrugUpdate();
		}

		[[nodiscard]] Geom::Matrix3D& getPorj() {
			return projection;
		}

		void resize(const unsigned w, const unsigned h) override {
			width = static_cast<float>(w);
			height = static_cast<float>(h);
			root->setSize(static_cast<float>(w), static_cast<float>(h));

			//TODO apply margin with FBO, not directly
			// projection.setOrthogonal(-marginX, -marginY, static_cast<float>(w) + marginX * 2.0f, static_cast<float>(h) + marginY * 2.0f);
			projection.setOrthogonal(0, 0, static_cast<float>(w), static_cast<float>(h));
		}

		[[nodiscard]] float getMarginX() const {
			return marginX;
		}

		[[nodiscard]] float getMarginY() const {
			return marginY;
		}

		[[nodiscard]] float getWidth() const {
			return width;
		}

		[[nodiscard]] float getHeight() const {
			return height;
		}

	virtual void render() const {
			root->draw();
		}

		void onDoubleClick(const int id) {
			if(currentCursorFocus == nullptr)return;
			doubleClickAction.set(cursorPos, id);
			currentCursorFocus->getInputListener().fire(doubleClickAction);
		}

		void onPress(const int id) {
			if(currentCursorFocus == nullptr)return;
			pressAction.set(cursorPos, id);
			currentCursorFocus->getInputListener().fire(pressAction);
			pressedMouseButtons[id] = true;
		}

		void onRelease(const int id) {
			if(currentCursorFocus == nullptr)return;
			releaseAction.set(cursorPos, id);
			currentCursorFocus->getInputListener().fire(releaseAction);
			pressedMouseButtons[id] = false;
		}

		[[nodiscard]] bool onDrug(const int id = 0) const {
			return pressedMouseButtons[id] && currentCursorFocus != nullptr;
		}

		void onDrugUpdate() const {
			if(currentCursorFocus == nullptr)return;
			for(int i = 0; i < Ctrl::MOUSE_BUTTON_COUNT; ++i) {
				if(onDrug(i)) {
					drugAction.set(cursorVel, i);
					currentCursorFocus->getInputListener().fire(drugAction);
				}
			}
		}

	protected:
		void inform(int keyCode, int action, int mods) override {

		}

		void setEnter(const Elem* elem) {
			if(elem == currentCursorFocus)return;

			if(currentCursorFocus != nullptr) {
				exboundAction.set(cursorPos);
				currentCursorFocus->getInputListener().fire(exboundAction);
			}

			currentCursorFocus = elem;

			if(currentCursorFocus != nullptr) {
				inboundAction.set(cursorPos);
				currentCursorFocus->getInputListener().fire(inboundAction);
			}
		}

	};
}
