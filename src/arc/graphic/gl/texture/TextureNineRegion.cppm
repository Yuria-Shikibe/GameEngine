export module GL.Texture.TextureNineRegion;

import std;
import Math;
import GL.Texture.TextureRegionRect;
import GL.Texture;
import Geom.Shape.Rect_Orthogonal;
import RuntimeException;
import Geom.Vector2D;

using Rect = Geom::Shape::OrthoRectFloat;
using HardRect = Geom::Shape::OrthoRectUInt;

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

		Geom::Vec2 bottomLeftSize{};
		Geom::Vec2 topRightSize{};


		[[nodiscard]] TextureNineRegion() = default;

		[[nodiscard]] TextureNineRegion(const TextureRegionRect* const rect, HardRect&& innerBound) {
			loadFrom(rect, std::forward<HardRect>(innerBound));
		}

		void loadFrom(const TextureRegionRect* const rect, HardRect&& innerBound) {
			const HardRect totalBound{
				Math::round<unsigned>(rect->u00() * rect->getData()->getWidth()),
				Math::round<unsigned>(rect->v00() * rect->getData()->getHeight()),
				Math::round<unsigned>(rect->getWidth()),
				Math::round<unsigned>(rect->getHeight())
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

			const unsigned int
				srcX = totalBound.getSrcX(),
				srcY = totalBound.getSrcY(),
				bottomLeft_W = innerBound.getSrcX() - srcX,
				bottomLeft_H = innerBound.getSrcY() - srcY,
				inner_W = innerBound.getWidth(),
				inner_H = innerBound.getHeight(),
				topRight_W = totalBound.getEndX() - innerBound.getEndX(),
				topRight_H = totalBound.getEndY() - innerBound.getEndY();

			regions[ID_center     ].fetchIntoCurrent(innerBound);

			regions[ID_right      ].fetchIntoCurrent(HardRect{innerBound.getEndX(), innerBound.getSrcY(), topRight_W, inner_H});
			regions[ID_top        ].fetchIntoCurrent(HardRect{innerBound.getSrcX(), innerBound.getEndY(), inner_W, topRight_H});
			regions[ID_left       ].fetchIntoCurrent(HardRect{srcX, innerBound.getSrcY(), bottomLeft_W, inner_H});
			regions[ID_bottom     ].fetchIntoCurrent(HardRect{innerBound.getSrcX(), srcY, inner_W, bottomLeft_H});

			regions[ID_topRight   ].fetchIntoCurrent(HardRect{innerBound.getEndX(), innerBound.getEndY(), topRight_W, topRight_H});
			regions[ID_topLeft    ].fetchIntoCurrent(HardRect{srcX, innerBound.getEndY(), bottomLeft_W, topRight_H});
			regions[ID_bottomLeft ].fetchIntoCurrent(HardRect{srcX, srcY, bottomLeft_W, bottomLeft_H});
			regions[ID_bottomRight].fetchIntoCurrent(HardRect{innerBound.getEndX(), srcY, topRight_W, bottomLeft_H});

			for(TextureRegionRect& region : regions) {
				if(region.getWidth() * region.getHeight() < 0.2f) {
					region.setData(nullptr);
				}
			}

			innerSize.set(inner_W, inner_H);
			bottomLeftSize.set(bottomLeft_W, bottomLeft_H);
			topRightSize.set(topRight_W, topRight_H);
		}

		[[nodiscard]] float getSrcX() const {
			return -bottomLeftSize.x;
		}

		[[nodiscard]] float getSrcY() const {
			return -bottomLeftSize.y;
		}

		[[nodiscard]] float getEndX() const {
			return innerSize.x + topRightSize.x;
		}

		[[nodiscard]] float getEndY() const {
			return innerSize.y + topRightSize.y;
		}

		[[nodiscard]] float getWidth() const {
			return innerSize.x + topRightSize.x + bottomLeftSize.x;
		}

		[[nodiscard]] float getHeight() const {
			return innerSize.y + topRightSize.y + bottomLeftSize.y;
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

		void render_RelativeExter(const Rect& rect) const{
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
