module;

#include <irrKlang.h>

export module UI.Elem;

export import UI.Flags;
export import UI.Align;
export import UI.Action;
export import UI.CursorType;
export import UI.Align;
import UI.SeperateDrawable;
import Event;
import Math;
import Geom.Vector2D;
import Graphic.Color;
import Geom.Rect_Orthogonal;
import ext.RuntimeException;
import Assets.Bundle;
export import OS.Ctrl.Bind.Constants;

import std;

export namespace UI {
	using SoundSource = irrklang::ISoundSource*;

	struct WidgetDrawer;
	class Group;
	class Table;
	class Root;

	constexpr float DisableAutoTooltip = -1.0f;
	constexpr float DefTooltipHoverTime = 25.0f;

	struct TooltipBuilder{
		enum struct FollowTarget : unsigned char{
			none,
			cursor,
			parent,
		};

		FollowTarget followTarget{FollowTarget::none};
		float minHoverTime{DefTooltipHoverTime};
		bool useStaticTime{true};
		bool autoRelease{true};
		Geom::Vec2 offset{};
		Align::Layout followTargetAlign{Align::Layout::bottom_left};
		Align::Layout tooltipSrcAlign{Align::Layout::top_left};

		std::function<void(Table&)> builder{};

		[[nodiscard]] explicit operator bool() const noexcept{
			return static_cast<bool>(builder);
		}

		[[nodiscard]] bool autoBuild() const noexcept{
			return minHoverTime >= 0.0f;
		}
	};

	using TooltipFollowTarget = TooltipBuilder::FollowTarget;
}

export namespace UI {
	struct LayoutCell;

	using Rect = Geom::OrthoRectFloat;

	class Elem : public SeperateDrawable{
		inline static std::vector<std::unique_ptr<Elem>> emptyElems{};

	public:
		int PointCheck{0};

		~Elem() noexcept override{
			releaseAllFocus();
		}

		[[nodiscard]] Elem(){
			Elem::applyDefDrawer();
		}

		friend ::UI::LayoutCell;

	protected:

		/**
		 * \brief The srcx, srcy is relative to its parent.
		 */
		Rect bound{};

		Group* parent{nullptr};
		mutable ::UI::Root* root{nullptr};

		//TODO is this necessary?
		// std::unordered_set<Widget*> focusTarget{};

		Event::EventManager inputListener{
			Event::indexOf<UI::MouseActionPress>(),
			Event::indexOf<UI::MouseActionRelease>(),
			Event::indexOf<UI::MouseActionDrag>(),
			Event::indexOf<UI::MouseActionDoubleClick>(),
			Event::indexOf<UI::MouseActionScroll>(),
			Event::indexOf<UI::CurosrInbound>(),
			Event::indexOf<UI::CurosrExbound>(),
		};

		TouchbilityFlags touchbility = TouchbilityFlags::disabled;

		bool fillParentX{false};
		bool fillParentY{false};

		bool endRow{false};

		bool visiable{true};

		/** Whether this element is being pressed*/
		bool pressed{false};
		/** Whether this element is in disabled state*/
		bool disabled{false};
		/** Whether this element is in active state*/
		bool activated{false};

		bool sleep{false};

		bool dropFocusAtCursorQuitBound{true};

		Geom::Vec2 absoluteSrc{};
		Geom::Vec2 minimumSize{};
		Geom::Vec2 maximumSize{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};

		WidgetDrawer* drawer{nullptr};

		Align::Spacing border{};

		std::queue<std::unique_ptr<Action<Elem>>> actions{};

		ChangeSignal lastSignal{ChangeSignal::notifyNone};
		mutable bool layoutChanged{false};

		// bool requiresLayout{false};

		[[nodiscard]] float clampTargetWidth(const float w) const{
			return  Math::clamp(w, minimumSize.x, maximumSize.x);
		}

		[[nodiscard]] float clampTargetHeight(const float h) const{
			return  Math::clamp(h, minimumSize.y, maximumSize.y);
		}

		Table* hoverTableHandle{nullptr};
		TooltipBuilder tooltipbuilder{};

	public:
		Elem(const Elem& other) = delete;

		Elem(Elem&& other) noexcept = default;

		Elem& operator=(const Elem& other) = delete;

		Elem& operator=(Elem&& other) noexcept = default;

		std::string name{""};

		std::function<bool()> visibilityChecker{nullptr};
		std::function<bool()> disableChecker{nullptr};
		std::function<bool()> activatedChecker{nullptr};

		std::function<void()> appendUpdator{nullptr};

		Graphic::Color color{1.0f, 1.0f, 1.0f, 1.0f};

		mutable Graphic::Color tempColor{0.0f, 0.0f, 0.0f, 0.0f};
		mutable float maskOpacity = 1.0f;

		float selfMaskOpacity = 1.0f;

		std::any animationData{};

		[[nodiscard]] constexpr Geom::Vec2 getMinimumSize() const noexcept{ return minimumSize; }

		constexpr void setMinimumSize(const Geom::Vec2 minimumSize) noexcept{
			this->minimumSize = minimumSize;
			setWidth(Math::max(minimumSize.x, getWidth()));
			setHeight(Math::max(minimumSize.y, getHeight()));
		}

		[[nodiscard]] constexpr Geom::Vec2 getMaximumSize() const noexcept{ return maximumSize; }

		constexpr void setMaximumSize(const Geom::Vec2 maximumSize) noexcept{
			this->maximumSize = maximumSize;
			setWidth(Math::min(maximumSize.x, getWidth()));
			setHeight(Math::min(maximumSize.y, getHeight()));
		}

		[[nodiscard]] virtual bool isVisiable() const noexcept{return visiable;}

		//TODO rename this shit
		[[nodiscard]] constexpr bool shouldDropFocusAtCursorQuitBound() const noexcept{
			return dropFocusAtCursorQuitBound;
		}

		constexpr void setDropFocusAtCursorQuitBound(const bool quitInboundFocus) noexcept{ this->dropFocusAtCursorQuitBound = quitInboundFocus; }

		[[nodiscard]] bool isPressed() const noexcept{return pressed;}

		[[nodiscard]] bool isSleep() const noexcept{ return sleep; }

		void setSleep(const bool sleep) noexcept{ this->sleep = sleep; }

		void passSound(SoundSource sound) const;

		/**
		 * @param elem Element To Fill This(it's parent)
		 * @return Expected Bound Of This Child Element
		 */
		virtual Rect getFilledChildrenBound(Elem* elem) const noexcept{
			Rect bound = elem->bound;

			bound.setSrc(border.bot_lft());

			if(elem->fillParentX){
				bound.setWidth(getValidWidth());
			}

			if(elem->fillParentY){
				bound.setHeight(getValidHeight());
			}

			return bound;
		}

		virtual bool layout_tryFillParent() noexcept;

		[[nodiscard]] bool isFillParentX() const noexcept{
			return fillParentX;
		}

		[[nodiscard]] bool isFillParentY() const noexcept{
			return fillParentY;
		}

		constexpr void notifyLayoutChanged() const noexcept{
			layoutChanged = true;
		}

		virtual void layout() {
			layout_tryFillParent();

			layoutChanged = false;
		}

		virtual void applySettings() noexcept{
			//TODO what is this used for... I forgot
		}

		virtual bool isIgnoreLayout() const noexcept{
			return !visiable;
		}

		/**
		 * @brief Used to create blur effect, this is draw as a mask
		 */
		void drawBase() const override;
		void drawBase(Rect rect) const;

		void draw() const override;

		virtual void drawContent() const {}

		virtual void setVisible(const bool val) noexcept{
			visiable = val;
		}

		virtual void setRoot(Root* const root) noexcept{
			this->root = root;
		}

		[[nodiscard]] UI::Root* getRoot() const noexcept{ return root; }

		virtual void applyDefDrawer() noexcept;

		void setTooltipBuilder(const TooltipBuilder& hoverTableBuilder) noexcept{
			this->tooltipbuilder = hoverTableBuilder;
		}

		void setTooltipBuilder(TooltipBuilder&& hoverTableBuilder) noexcept{
			this->tooltipbuilder = std::move(hoverTableBuilder);
		}

		[[nodiscard]] const TooltipBuilder& getTooltipBuilder() const noexcept{ return tooltipbuilder; }

		void updateHoverTableHandle(Table* handle) noexcept{
			this->hoverTableHandle = handle;
		}

		void setFillparentX(const bool val = true) noexcept{
			if(val != fillParentX)changed(UI::ChangeSignal::notifySubs);

			fillParentX = val;
		}

		void setFillparentY(const bool val = true) noexcept{
			if(val != fillParentY)changed(UI::ChangeSignal::notifySubs);

			fillParentY = val;
		}

		void setFillparent(const bool fillX = true, const bool fillY = true) noexcept{
			setFillparentX(fillX);
			setFillparentY(fillY);
		}

		[[nodiscard]] constexpr bool touchDisabled() const noexcept{
			return touchbility == TouchbilityFlags::disabled;
		}

		[[nodiscard]] constexpr TouchbilityFlags getTouchbility() const noexcept{
			return touchbility;
		}

		constexpr void setTouchbility(const TouchbilityFlags flag) noexcept{
			this->touchbility = flag;
		}

		[[nodiscard]] constexpr float getBorderWidth() const noexcept{return border.getWidth();}

		[[nodiscard]] constexpr float getBorderHeight() const noexcept{return border.getHeight();}

		[[nodiscard]] constexpr float getValidWidth() const noexcept{return Math::clampPositive(getWidth() - getBorderWidth());}

		[[nodiscard]] constexpr float getValidHeight() const noexcept{return Math::clampPositive(getHeight() - getBorderHeight());}

		virtual void drawStyle() const;

		[[nodiscard]] Group* getParent() const;

		void setDrawer(WidgetDrawer* drawer);

		virtual void setEmptyDrawer();

		/**
		 * @return The former parent group
		 */
		Group* setParent(Group* parent);

		virtual void callRemove();

		[[nodiscard]] constexpr bool isEndingRow() const noexcept{return endRow;}

		constexpr void setEndRow(const bool end) noexcept {endRow = end;}

		// [[nodiscard]] const std::unordered_set<Widget*>& getFocus() const {return focusTarget;}
		//
		// [[nodiscard]] std::unordered_set<Widget*>& getFocus() {return focusTarget;}
		//
		// virtual void addFocusTarget(Widget* const target) {
		// 	focusTarget.insert(target);
		// }
		//
		// virtual void removeFocusTarget(Widget* const target) {
		// 	focusTarget.erase(target);
		// }

		void setSrc(const Geom::Vec2 src) noexcept{
			setSrc(src.x, src.y);
		}

		void setSrc(const float x, const float y) noexcept{
			if(bound.getSrcX() == x && bound.getSrcY() == y)return;
			bound.setSrc(x, y);
			changed(UI::ChangeSignal::notifyAll);
		}

		/** @return true if changed */
		virtual bool setWidth(float w) noexcept{
			w = clampTargetWidth(w);
			if(Math::equal(bound.getWidth(), w))return false;
			bound.setWidth(w);
			changed(UI::ChangeSignal::notifyAll);
			return true;
		}

		/** @return true if changed */
		virtual bool setHeight(float h) noexcept{
			h = clampTargetHeight(h);
			if(Math::equal(bound.getHeight(), h))return false;
			bound.setHeight(h);
			changed(UI::ChangeSignal::notifyAll);
			return true;
		}

		/** @return true if changed */
		bool setSize(const float w, const float h) {
			//Cautious for short circuit evaluation
			bool isChanged = false;
			isChanged |= setWidth(w);
			isChanged |= setHeight(h);
			return isChanged;
		}

		/** @return true if changed */
		bool setSize(const float s) {
			return setSize(s, s);
		}

		//virtual float getIdealWidth() const noexcept {return bound.getWidth();}

		//virtual float getIdealHeight() const noexcept {return bound.getHeight();}

		[[nodiscard]] constexpr Rect getBound() const noexcept {return bound;}

		[[nodiscard]] constexpr Rect getValidBound() const noexcept {return {border.left, border.bottom, getValidWidth(), getValidHeight()};}

		[[nodiscard]] constexpr Geom::Vec2 getValidSize() const noexcept {return {getValidWidth(), getValidHeight()};}

		virtual void calAbsoluteSrc(Elem* parent) noexcept{
			Geom::Vec2 vec = parent ? parent->absoluteSrc : Geom::ZERO;

			vec.add(bound.getSrc());
			if(vec == absoluteSrc)return;
			absoluteSrc.set(vec);
			calAbsoluteChildren();
		}

		virtual void calAbsoluteChildren() noexcept {}

		[[nodiscard]] Geom::Vec2 getAbsSrc() const noexcept{
			return absoluteSrc;
		}

		void setAbsSrc(const Geom::Vec2 src) noexcept{
			if(absoluteSrc != src){
				absoluteSrc = src;
				changed(UI::ChangeSignal::notifyAll);
			}
		}

		// TODO json srl support
		// virtual int elemSerializationID() noexcept{
		// 	return 0;
		// }

		[[nodiscard]] auto& getInputListener() noexcept{return inputListener;}
		[[nodiscard]] auto& getInputListener() const noexcept{return inputListener;}

		[[nodiscard]] constexpr bool hasChanged() const noexcept{return layoutChanged;}

		/**
		 * @brief
		 * This is a post signal function.
		 * After change is called, layout should be called in the next update to handle the change.
		 */
		void changed(const ChangeSignal direction, const ChangeSignal removal = ChangeSignal::notifyNone) noexcept{
			lastSignal = lastSignal + direction - removal;
		}

		virtual void postChanged() noexcept;

		void overrideChanged(const bool val, const ChangeSignal removal = ChangeSignal::notifyNone) noexcept{
			layoutChanged = val;
			lastSignal = removal;
		}

		virtual void toString(std::ostream& os, const int depth) const {
			//TODO tree print support
		}

		virtual void setDisabled(const bool disabled) noexcept{
			this->disabled = disabled;
		}

		[[nodiscard]] std::function<bool()>& getActivatedChecker() noexcept{ return activatedChecker; }

		void setActivatedChecker(const std::function<bool()>& activatedChecker) noexcept{
			this->activatedChecker = activatedChecker;
		}

		virtual void setActivated(const bool activated) noexcept{
			this->activated = activated;
		}

		[[nodiscard]] bool isActivated() const noexcept{ return activated; }

		virtual void update(const float delta){
			if(visibilityChecker) [[unlikely]] setVisible(visibilityChecker());
			if(disableChecker) [[unlikely]] setDisabled(disableChecker());
			if(activatedChecker) [[unlikely]] setActivated(activatedChecker());
			if(appendUpdator) [[unlikely]] appendUpdator();


			for(float actionDelta = delta; !actions.empty();){
				const auto& current = actions.front();

				actionDelta = current->update(actionDelta, this);

				if(actionDelta >= 0) [[unlikely]] {
					actions.pop();
				}else{
					break;
				}
			}
		}

		[[nodiscard]] auto& getActions() const noexcept{ return actions; }
		[[nodiscard]] auto& getActions() noexcept{ return actions; }

		template <Concepts::Derived<Action<Elem>> ActionType, typename ...T>
		void pushAction(T&&... args){
			actions.push(std::make_unique<ActionType>(std::forward<T>(args)...));
		}

		template<Concepts::Derived<Action<Elem>> ...ActionType>
		void pushActions(std::unique_ptr<ActionType>&& ...actionArgs){
			std::array<Action<Elem>, sizeof...(actionArgs)> arr = {actionArgs...};
			actions.push_range(arr);
		}

		virtual const std::vector<std::unique_ptr<Elem>>& getChildren() const noexcept{
			return emptyElems;
		}

		virtual bool hasChildren() const noexcept {return false;}

		[[nodiscard]] constexpr bool isInteractable() const noexcept{
			return (touchbility == TouchbilityFlags::enabled || static_cast<bool>(tooltipbuilder)) && visiable;
		}

		[[nodiscard]] constexpr bool isQuietInteractable() const noexcept{
			return (touchbility != TouchbilityFlags::enabled && static_cast<bool>(tooltipbuilder)) && visiable;
		}

		virtual bool hintInbound_validToParent(const Geom::Vec2 screenPos) noexcept{
			return isInbound(screenPos);
		}

		[[nodiscard]] virtual bool isInbound(Geom::Vec2 screenPos) const noexcept;

		bool isFocusedKeyInput() const noexcept;

		bool isFocusedScroll() const noexcept;

		bool isCursorInbound() const noexcept;

		void setFocusedKey(bool focus) noexcept;

		void setFocusedScroll(bool focus) noexcept;

		void releaseAllFocus() const noexcept;

		Geom::Vec2 getCursorPos() const noexcept;

		virtual CursorType getCursorType() const noexcept{
			if(touchbility != TouchbilityFlags::enabled){
				return tooltipbuilder ? CursorType::regular_tip : CursorType::regular;
			}else{
				return tooltipbuilder ? CursorType::clickable_tip : CursorType::clickable;
			}
		}

		[[nodiscard]] constexpr float drawSrcX() const noexcept{return absoluteSrc.x;}

		[[nodiscard]] constexpr float drawSrcY() const noexcept{return absoluteSrc.y;}

		[[nodiscard]] constexpr float getWidth() const noexcept{return bound.getWidth();}

		[[nodiscard]] constexpr float getHeight() const noexcept{return bound.getHeight();}

		[[nodiscard]] const Align::Spacing& getBorder() const noexcept{ return border; }
		[[nodiscard]] Align::Spacing& getBorder() noexcept{ return border; }

		void setBorderZero() noexcept {setBorder(0.0f);}

		void setBorder(const Align::Spacing margin) noexcept{
			if(margin != this->border){
				this->border = margin;
				changed(UI::ChangeSignal::notifySubs);
			}
		}

		void setBorder(const float val) noexcept{
			if(border != val){
				this->border.set(val);
				changed(UI::ChangeSignal::notifySubs);
			}
		}

		bool isTrivialElem() const noexcept{
			return appendUpdator == nullptr && disableChecker == nullptr && activatedChecker == nullptr && visibilityChecker == nullptr;
		}

		bool keyDown(const int code, const int action, const int mode = Ctrl::Mode::Ignore) const;

		/**
		 * @return True if there is nothing can do, then transfer the control
		 */
		virtual bool onEsc(){
			return true;
		}

		void buildTooltip();

		[[nodiscard]] std::string_view getBundleEntry(std::string_view key, bool fromUICategory = true) const;
		[[nodiscard]] Assets::Bundle& getBundles(bool fromUICategory = true) const;

	protected:
		virtual void childrenCheck(const Elem* ptr) {
			throw ext::IllegalArguments{"Labels shouldn't have children!"};
		}
	};
}
