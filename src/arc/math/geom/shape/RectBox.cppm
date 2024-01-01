module;

export module Geom.Shape.RectBox;

import Geom.Point2D;
import Geom.Vector2D;
import Geom.Shape.Rect_Orthogonal;

import Math;
import <algorithm>;
import <cmath>;
import <array>;
import <limits>;

export namespace Geom {
	struct RectBox {
		using VertGroup = std::array<Geom::PointF2D, 4>;
		/**
		 * \brief x for rect width, y for rect height
		 */
		Geom::PointF2D edgeLength{};

		/**
		 * \brief Geomtery Center
		 */
		Geom::PointF2D originPoint{};
		/**
		 * \brief Center To Bottom-Left Offset
		 */
		Geom::PointF2D offset{};

		/**
		 * \brief Rect Rotation
		 */
		float rotation{0};

		/**
		 * \brief Exported Vert [bottom-left, bottom-right, top-right, top-left]
		 */
		Geom::PointF2D vert0_dynamic{};
		Geom::PointF2D vert1_dynamic{};
		Geom::PointF2D vert2_dynamic{};
		Geom::PointF2D vert3_dynamic{};


		/**
		 * \brief Normal Vector
		 */
		Geom::PointF2D normalU{};
		Geom::PointF2D normalV{};

		Geom::Shape::OrthoRectFloat maxOrthoBound{};

		void setSize(const float w, const float h) {
			edgeLength.set(w, h);
		}

		void update() {
			const float cos = Math::cosDeg(rotation);
			const float sin = Math::sinDeg(rotation);

			vert0_dynamic.set(offset).rotate(cos, sin);
			vert1_dynamic.set(edgeLength.x, 0).rotate(cos, sin);
			vert3_dynamic.set(0, edgeLength.y).rotate(cos, sin);
			vert2_dynamic = vert1_dynamic + vert3_dynamic;

			normalU = vert3_dynamic;
			normalV = vert1_dynamic;

			vert0_dynamic += originPoint;
			vert1_dynamic += vert0_dynamic;
			vert2_dynamic += vert0_dynamic;
			vert3_dynamic += vert0_dynamic;

			auto [xMin, xMax] = std::minmax({vert0_dynamic.x, vert1_dynamic.x, vert2_dynamic.x, vert3_dynamic.x});
			auto [yMin, yMax] = std::minmax({vert0_dynamic.y, vert1_dynamic.y, vert2_dynamic.y, vert3_dynamic.y});

			maxOrthoBound.setVert(xMin, yMin, xMax, yMax);
		}

		[[nodiscard]] bool axisOverlap(const RectBox& other, const Geom::PointF2D& axis) const {
			float box1_min   = vert0_dynamic.dot(axis);
			float box1_max   = box1_min;
			float projection = vert1_dynamic.dot(axis);
			box1_min         = std::min(box1_min, projection);
			box1_max         = std::max(box1_max, projection);
			projection       = vert2_dynamic.dot(axis);
			box1_min         = std::min(box1_min, projection);
			box1_max         = std::max(box1_max, projection);
			projection       = vert3_dynamic.dot(axis);
			box1_min         = std::min(box1_min, projection);
			box1_max         = std::max(box1_max, projection);

			float other_min = other.vert0_dynamic.dot(axis);
			float other_max = other_min;
			projection      = other.vert1_dynamic.dot(axis);
			other_min       = std::min(other_min, projection);
			other_max       = std::max(other_max, projection);
			projection      = other.vert2_dynamic.dot(axis);
			other_min       = std::min(other_min, projection);
			other_max       = std::max(other_max, projection);
			projection      = other.vert3_dynamic.dot(axis);
			other_min       = std::min(other_min, projection);
			other_max       = std::max(other_max, projection);

			return box1_max >= other_min && other_max >= box1_min;
		}

		[[nodiscard]] bool overlapExact(const RectBox& other) const {
			return
				axisOverlap(other, normalU) && axisOverlap(other, normalV) &&
				axisOverlap(other, other.normalU) && axisOverlap(other, other.normalV);
		}

		[[nodiscard]] bool overlapRough(const RectBox& other) const {
			return
				maxOrthoBound.overlap(other.maxOrthoBound);
		}

		[[nodiscard]] VertGroup verts() const {
			return VertGroup{vert0_dynamic, vert1_dynamic, vert2_dynamic, vert3_dynamic};
		}
	};
}
