module;

export module UI.Flags;

import Event;
import Geom.Vector2D;

export namespace UI {
	using Event::EventType;

	enum class MouseInbound {
		inbound = 1, outbound = 0
	};

	struct MouseAction : EventType, Geom::Vec2 {
		using Vector2D::x;
		using Vector2D::y;

		int key{0};
		int mode{0};

		[[nodiscard]] MouseAction() = default;

		[[nodiscard]] MouseAction(const float x, const float y)
			: Vector2D(x, y) {
		}

		void set(const float x, const float y, const int id) {
			Geom::Vec2::set(x, y);
			key = id;
		}

		void set(const Geom::Vec2& pos, const int id, const int mode) {
			Geom::Vec2::operator=(pos);
			this->key = id;
			this->mode = mode;
		}
	};

	struct MouseActionScroll final : EventType, Geom::Vec2 {
		[[nodiscard]] MouseActionScroll() = default;

		[[nodiscard]] MouseActionScroll(const float x, const float y)
			: Geom::Vec2(x, y) {
		}
	};

	/**
	 * @brief it's (x, y) is for cursor velocity
	 */
	struct MouseActionDrag final : MouseAction  {
		Geom::Vec2 begin{};
		Geom::Vec2 end{};

		[[nodiscard]] Geom::Vec2 getRelativeMove() const{
			return end - begin;
		}

		[[nodiscard]] MouseActionDrag() = default;

		[[nodiscard]] MouseActionDrag(const float x, const float y)
			: MouseAction(x, y){
		}
	};

	struct MouseActionPress final : MouseAction  {
		[[nodiscard]] MouseActionPress() = default;

		[[nodiscard]] MouseActionPress(const float x, const float y)
			: MouseAction(x, y) {
		}
	};

	struct MouseActionRelease final : MouseAction  {
		[[nodiscard]] MouseActionRelease() = default;

		[[nodiscard]] MouseActionRelease(const float x, const float y)
			: MouseAction(x, y) {
		}
	};

	struct MouseActionDoubleClick final : MouseAction  {
		[[nodiscard]] MouseActionDoubleClick() = default;

		[[nodiscard]] MouseActionDoubleClick(const float x, const float y)
			: MouseAction(x, y) {
		}
	};

	struct CurosrInbound final : EventType, Geom::Vec2 {
		[[nodiscard]] CurosrInbound() = default;

		[[nodiscard]] CurosrInbound(const float x, const float y)
			: Geom::Vec2(x, y) {
		}
	};

	struct CurosrExbound final : EventType, Geom::Vec2 {
		[[nodiscard]] CurosrExbound() = default;

		[[nodiscard]] CurosrExbound(const float x, const float y)
			: Geom::Vec2(x, y) {
		}
	};

	/**
	 * \brief shouldn't be used, really
	 */
	struct LongClick final : EventType, Geom::Vec2  {
		using Geom::Vec2::x;
		using Geom::Vec2::y;

		[[nodiscard]] LongClick() = default;

		[[nodiscard]] LongClick(const float x, const float y)
			: Geom::Vec2(x, y) {
		}
	};

	enum class TouchbilityFlags : unsigned char{
		disabled = 0,
		enabled = 1,
		childrenOnly = 2
	};

	enum class ChangeSignal : unsigned char{
		notifyNone = 0b0000'0000,
		notifySelf = 0b0000'0001,
		notifyChildrenOnly = 0b0000'0010,
		notifyParentOnly = 0b0000'0100,
		notifyAll = notifySelf | notifyChildrenOnly | notifyParentOnly,
		notifySubs = notifySelf | notifyChildrenOnly,
		notifySupers = notifySelf | notifyParentOnly,
	};

	// typedef unsigned char TouchbilityState;
	//
	// enum ElemEventFlags : unsigned char{
	// 	caresNone = 0b0000'0001,
	// 	caresImportant = 0b0000'0010,
	// 	caresAll = 0b0000'0100,
	//
	// 	quiet = 0b0001'0000,
	// 	sendImportant = 0b0010'0000,
	// 	sendAll = 0b0100'0000,
	// };
	//
	// typedef unsigned char ElemEventState;
	//
	// enum ChangedType : unsigned short{
	// 	width = 0b0000'0000'0000'0001,
	// 	height = 0b0000'0000'0000'0010,
	//
	// 	srcX = 0b0000'0000'0000'0100,
	// 	srcY = 0b0000'0000'0000'1000,
	//
	// 	content = 0b0000'0000'0001'0000,
	// 	color = 0b0000'0000'0010'0000,
	//
	// 	/**
	// 	 * \brief When @link visibility @endlink changed, a UI::Elem 's Color.alpha, srcX, srcY, width, height -> 0.
	// 	 */
	// 	visibility = 0b0000'0000'0111'1111,
	// 	/**
	// 	 * \brief When @link valid @endlink changed, a UI::Elem 's Color should become darker or sth like red to show that they are now unable to use.
	// 	 */
	// 	valid = 0b0000'0000'1000'0000 | color,
	//
	// 	touchbility = 0b0000'0001'0000'0000,
	//
	// 	layer = 0b0000'0010'0000'0000,
	// };
	//
	// typedef unsigned short ChangedTypeFlags;
	//
	// struct Changed : EventType {
	// 	ChangedTypeFlags flags = 0;
	//
	// 	[[nodiscard]] explicit Changed(const ChangedTypeFlags flags)
	// 		: flags(flags) {
	// 	}
	//
	// 	[[nodiscard]] Changed() = default;
	//
	// 	[[nodiscard]] bool contains(const ChangedType type) const {
	// 		return flags | type;
	// 	}
	//
	// 	void add(const ChangedType type) {
	// 		flags |= type;
	// 	}
	//
	// 	void remove(const ChangedType type) {
	// 		flags &= ~type;
	// 	}
	// };
	//
	// struct ParentChanged final : Changed{};
	// struct ChildrenChanged final : Changed{};
}

export bool operator&(UI::ChangeSignal l, UI::ChangeSignal r){
	return static_cast<unsigned>(l) & static_cast<unsigned>(r);
}

export UI::ChangeSignal operator+(UI::ChangeSignal l, UI::ChangeSignal r){
	return static_cast<UI::ChangeSignal>(static_cast<unsigned>(l) | static_cast<unsigned>(r));
}

export UI::ChangeSignal operator-(const UI::ChangeSignal l, const UI::ChangeSignal r){
	return static_cast<UI::ChangeSignal>(static_cast<unsigned>(l) & ~static_cast<unsigned>(r));
}
