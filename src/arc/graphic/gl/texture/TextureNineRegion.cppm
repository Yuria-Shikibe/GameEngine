export module GL.Texture.TextureNineRegion;

import std;
import Math;
import GL.Texture.TextureRegionRect;
import GL.Texture;
import Geom.Rect_Orthogonal;
import ext.RuntimeException;
import Geom.Vector2D;
import UI.Align;

using Rect = Geom::OrthoRectFloat;
using HardRect = Geom::OrthoRectInt;

export namespace GL {
	/**
	 * \brief
	 *	When render this TextrueRegion, only the Inner(Center) Part should be stretched, and nearby edge parts scale in its axis, and corner parts with no scale!
	 *
	 * @code
	 * |                         |                   |
	 * |               ID: 6     |       ID: 2       |    ID : 5
	 * |                         |                   |   Top-Right Part
	 * |                         |                   |
	 * |              -----------+-------------------+-----------
	 * |                         |                   |
	 * |                         |                   |
	 * |               ID: 3     |       ID: 0       |    ID: 1
	 * |                         |     Inner Part    |
	 * |                         |                   |
	 * |                         |                   |
	 * |              -----------+-------------------+-----------
	 * |                         |                   |
	 * |               ID: 7     |       ID: 4       |    ID: 8
	 * |    Bottom-Left Part     |                   |
	 * |                         |                   |
	 * @endcode
	 */
	class TextureNineRegion {
	public:
		static constexpr int ID_center      = 0;
		static constexpr int ID_right       = 1;
		static constexpr int ID_top         = 2;
		static constexpr int ID_left        = 3;
		static constexpr int ID_bottom      = 4;
		static constexpr int ID_topRight    = 5;
		static constexpr int ID_topLeft     = 6;
		static constexpr int ID_bottomLeft  = 7;
		static constexpr int ID_bottomRight = 8;

		static constexpr int TotalRegions = 9;

		std::array<TextureRegionRect, TotalRegions> regions{};

		Geom::Vec2 innerSize{};

		Align::Spacing edge{};


		[[nodiscard]] TextureNineRegion() = default;

		[[nodiscard]] TextureNineRegion(const TextureRegionRect* const rect, HardRect&& innerBound) {
			loadFrom(rect, std::forward<HardRect>(innerBound));
		}

		void loadFrom(const TextureRegionRect* const rect, HardRect&& innerBound) {
			const HardRect totalBound{
				Math::round<int>(rect->u00() * static_cast<float>(rect->getData()->getWidth())),
				Math::round<int>(rect->v00() * static_cast<float>(rect->getData()->getHeight())),
				Math::round<int>(rect->getWidth()),
				Math::round<int>(rect->getHeight())
			};

			innerBound.move(totalBound.getSrcX(), totalBound.getSrcY());

			loadFrom(rect->getData(), totalBound, innerBound);
		}

		void loadFrom(const Texture* const rect, const HardRect& totalBound, const HardRect& innerBound) {
#ifdef _DEBUG
			if(
				totalBound.getSrcX() > innerBound.getSrcX() || totalBound.getSrcY() > innerBound.getSrcY() ||
				totalBound.getEndX() < innerBound.getEndX() || totalBound.getEndY() < innerBound.getEndY()
			) {
				throw ext::IllegalArguments{"NineRegion Receives An Inner Part Larger Than Exter Part!"};
			}
#endif

			for(TextureRegionRect& region : regions) {
				region.setData(rect);
			}

			const int
				srcX = totalBound.getSrcX(),
				srcY = totalBound.getSrcY(),
				left = innerBound.getSrcX() - srcX,
				bottom = innerBound.getSrcY() - srcY,
				innerWidth = innerBound.getWidth(),
				innerHeight = innerBound.getHeight(),
				right = totalBound.getEndX() - innerBound.getEndX(),
				top = totalBound.getEndY() - innerBound.getEndY();

			regions[ID_center     ].fetchIntoCurrent(innerBound);

			regions[ID_right      ].fetchIntoCurrent(HardRect{innerBound.getEndX(), innerBound.getSrcY(), right, innerHeight});
			regions[ID_top        ].fetchIntoCurrent(HardRect{innerBound.getSrcX(), innerBound.getEndY(), innerWidth, top});
			regions[ID_left       ].fetchIntoCurrent(HardRect{srcX, innerBound.getSrcY(), left, innerHeight});
			regions[ID_bottom     ].fetchIntoCurrent(HardRect{innerBound.getSrcX(), srcY, innerWidth, bottom});

			regions[ID_topRight   ].fetchIntoCurrent(HardRect{innerBound.getEndX(), innerBound.getEndY(), right, top});
			regions[ID_topLeft    ].fetchIntoCurrent(HardRect{srcX, innerBound.getEndY(), left, top});
			regions[ID_bottomLeft ].fetchIntoCurrent(HardRect{srcX, srcY, left, bottom});
			regions[ID_bottomRight].fetchIntoCurrent(HardRect{innerBound.getEndX(), srcY, right, bottom});

			for(TextureRegionRect& region : regions) {
				if(region.getWidth() * region.getHeight() < 0.2f) {
					region.setData(nullptr);
				}
			}

			innerSize.set(static_cast<float>(innerWidth), static_cast<float>(innerHeight));
			edge.set(
				static_cast<float>(left),
				static_cast<float>(right),
				static_cast<float>(bottom),
				static_cast<float>(top)
			);
		}

		[[nodiscard]] float getSrcX() const {
			return -edge.left;
		}

		[[nodiscard]] float getSrcY() const {
			return -edge.bottom;
		}

		[[nodiscard]] float getEndX() const {
			return innerSize.x + edge.right;
		}

		[[nodiscard]] float getEndY() const {
			return innerSize.y + edge.top;
		}

		[[nodiscard]] float getWidth() const {
			return innerSize.x + edge.getWidth();
		}

		[[nodiscard]] float getHeight() const {
			return innerSize.y + edge.getHeight();
		}

		[[nodiscard]] Rect bound() const {
			return Rect{getSrcX(), getSrcY(), getWidth(), getHeight()};
		}

		void set(const int id, const TextureRegionRect& region) {
			regions[id] = region;
		}

		/**
		 * \brief
		 * \param x Inner Src X
		 * \param y Inner Src Y
		 * \param width  Inner Width
		 * \param height  Inner Height
		 */
		void render_RelativeInner(const float x, const float y, const float width, const float height) const;

		void render_RelativeInner(const Rect& rect) const{
			render_RelativeInner(rect.getSrcX(), rect.getSrcY(), rect.getWidth(), rect.getHeight());
		}

		/**
		 * \brief
		 * \param x Exter Src X
		 * \param y Exter Src Y
		 * \param width  Exter Width
		 * \param height  Exter Height
		 */
		void render_RelativeExter(float x, float y, float width, float height) const;

		void render_RelativeExter(const Rect rect) const{
			render_RelativeExter(rect.getSrcX(), rect.getSrcY(), rect.getWidth(), rect.getHeight());
		}

		/**
		 * \brief This class shouldn't responsible for negative input!
		 * \param w Inner Part Width
		 * \param h Inner Part Height
		 */
		void setSize(const float w, const float h) {
			innerSize.set(w, h);
		}
	};
}
