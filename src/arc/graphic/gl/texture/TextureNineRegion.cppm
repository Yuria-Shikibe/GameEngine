//
// Created by Matrix on 2023/12/3.
//

module;

export module GL.Texture.TextureNineRegion;

import <array>;
import GL.Texture.TextureRegionRect;
import Geom.Shape.Rect_Orthogonal;
import Geom.Vector2D;

import Graphic.Draw;

using Rect = Geom::Shape::OrthoRectFloat;
using namespace Graphic;

export namespace GL {
	/**
	 * \brief
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
		static constexpr int ID_center = 0;
		static constexpr int ID_right = 1;
		static constexpr int ID_top = 2;
		static constexpr int ID_left = 3;
		static constexpr int ID_bottom = 4;
		static constexpr int ID_topRight = 5;
		static constexpr int ID_topLeft = 6;
		static constexpr int ID_bottomLeft = 7;
		static constexpr int ID_bottomRight = 8;

		static constexpr int TotalRegions = 9;

		std::array<TextureRegionRect, TotalRegions> regions{};

		Geom::Vector2D innerSize{};

		Geom::Vector2D bottomLeftSize{};
		Geom::Vector2D topRightSize{};

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
		void render_RelativeInner(const float x, const float y, const float width, const float height) const{
			Draw::rect(regions[ID_center], x, y, width, height);

			Draw::rect(regions[ID_right], x + width, y, topRightSize.x, height);
			Draw::rect(regions[ID_top], x, y + height, width, topRightSize.y);
			Draw::rect(regions[ID_left], x - bottomLeftSize.x, y, bottomLeftSize.x, height);
			Draw::rect(regions[ID_bottom], x, y - bottomLeftSize.y, width, bottomLeftSize.y);

			Draw::rect(regions[ID_topRight], x + width, y + height, topRightSize.x, topRightSize.y);
			Draw::rect(regions[ID_topLeft], x - bottomLeftSize.x, y + height, bottomLeftSize.x, topRightSize.y);
			Draw::rect(regions[ID_bottomLeft], x - bottomLeftSize.x, y - bottomLeftSize.y, bottomLeftSize.x, bottomLeftSize.y);
			Draw::rect(regions[ID_bottomRight], x + width, y - bottomLeftSize.y, topRightSize.x, bottomLeftSize.y);
		}

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
		void render_RelativeExter(const float x, const float y, const float width, const float height) const{
			Draw::rect(regions[ID_center], x + bottomLeftSize.x, y + bottomLeftSize.y, width - bottomLeftSize.x - topRightSize.x, height - bottomLeftSize.y - topRightSize.y);

			Draw::rect(regions[ID_right], x + width - topRightSize.x, y + bottomLeftSize.y, topRightSize.x, height - bottomLeftSize.y - topRightSize.y);
			Draw::rect(regions[ID_top], x + bottomLeftSize.x, y + height - topRightSize.y, width - bottomLeftSize.x - topRightSize.x, topRightSize.y);
			Draw::rect(regions[ID_left], x, y + bottomLeftSize.y, bottomLeftSize.x,  height - bottomLeftSize.y - topRightSize.y);
			Draw::rect(regions[ID_bottom], x + bottomLeftSize.x, y, width - bottomLeftSize.x - topRightSize.x,  bottomLeftSize.y);

			Draw::rect(regions[ID_topRight], x + width - topRightSize.x, y + height - topRightSize.y, topRightSize.x, topRightSize.y);
			Draw::rect(regions[ID_topLeft], x, y + height - topRightSize.y, bottomLeftSize.x, topRightSize.y);
			Draw::rect(regions[ID_bottomLeft], x, y, bottomLeftSize.x, bottomLeftSize.y);
			Draw::rect(regions[ID_bottomRight], x + width - topRightSize.x, y, topRightSize.x, bottomLeftSize.y);
		}

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
