module;

export module UI.Flags;

import Event;
import Geom.Vector2D;

export namespace UI {
	using Event::EventType;

	enum class MouseInbound {
		inbound = 1, outbound = 0
	};

	struct MouseAction : EventType, Geom::Vector2D {
		using Vector2D::x;
		using Vector2D::y;

		unsigned int buttonID{0};

		[[nodiscard]] MouseAction() = default;

		[[nodiscard]] MouseAction(const float x, const float y)
			: Vector2D(x, y) {
		}

		void set(const float x, const float y, const int id) {
			this->x = x;
			this->y = y;
			buttonID = id;
		}

		void set(const Vector2D& pos, const int id) {
			Geom::Vector2D::operator=(pos);
			buttonID = id;
		}
	};

	struct MouseActionDrug final : MouseAction  {
		[[nodiscard]] MouseActionDrug() = default;

		[[nodiscard]] MouseActionDrug(const float x, const float y)
			: MouseAction(x, y) {
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

	struct CurosrInbound final : Event::EventType, Geom::Vector2D {
		[[nodiscard]] CurosrInbound() = default;

		[[nodiscard]] CurosrInbound(const float x, const float y)
			: Geom::Vector2D(x, y) {
		}
	};

	struct CurosrExbound final : Event::EventType, Geom::Vector2D {
		[[nodiscard]] CurosrExbound() = default;

		[[nodiscard]] CurosrExbound(const float x, const float y)
			: Geom::Vector2D(x, y) {
		}
	};

	/**
	 * \brief shouldn't be used, really
	 */
	struct LongClick final : EventType, Geom::Vector2D  {
		using Vector2D::x;
		using Vector2D::y;

		[[nodiscard]] LongClick() = default;

		[[nodiscard]] LongClick(const float x, const float y)
			: Vector2D(x, y) {
		}
	};

	enum class TouchbilityFlags : unsigned char{
		disabled = 0,
		enabled = 1,
		childrenOnly = 2
	};

	typedef unsigned char TouchbilityState;

	enum ElemEventFlags : unsigned char{
		caresNone = 0b0000'0001,
		caresImportant = 0b0000'0010,
		caresAll = 0b0000'0100,

		quiet = 0b0001'0000,
		sendImportant = 0b0010'0000,
		sendAll = 0b0100'0000,
	};

	typedef unsigned char ElemEventState;

	enum ChangedType : unsigned short{
		width = 0b0000'0000'0000'0001,
		height = 0b0000'0000'0000'0010,

		srcX = 0b0000'0000'0000'0100,
		srcY = 0b0000'0000'0000'1000,

		content = 0b0000'0000'0001'0000,
		color = 0b0000'0000'0010'0000,

		/**
		 * \brief When @link visibility @endlink changed, a UI::Elem 's Color.alpha, srcX, srcY, width, height -> 0.
		 */
		visibility = 0b0000'0000'0111'1111,
		/**
		 * \brief When @link valid @endlink changed, a UI::Elem 's Color should become darker or sth like red to show that they are now unable to use.
		 */
		valid = 0b0000'0000'1000'0000 | color,

		touchbility = 0b0000'0001'0000'0000,

		layer = 0b0000'0010'0000'0000,
	};

	typedef unsigned short ChangedTypeFlags;

	struct Changed : EventType {
		ChangedTypeFlags flags = 0;

		[[nodiscard]] explicit Changed(const ChangedTypeFlags flags)
			: flags(flags) {
		}

		[[nodiscard]] Changed() = default;

		[[nodiscard]] bool contains(const ChangedType type) const {
			return flags | type;
		}

		void add(const ChangedType type) {
			flags |= type;
		}

		void remove(const ChangedType type) {
			flags &= ~type;
		}
	};

	struct ParentChanged final : Changed{};
	struct ChildrenChanged final : Changed{};
}
