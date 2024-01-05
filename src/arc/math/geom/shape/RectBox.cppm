module;

export module Geom.Shape.RectBox;

import Geom.Vector2D;
import Geom.Shape.Rect_Orthogonal;

import Math;
import <algorithm>;
import <cmath>;
import <array>;
import <limits>;

export namespace Geom {
	struct RectBox {
		using VertGroup = std::array<Vec2, 4>;
		/**
		 * \brief x for rect width, y for rect height, static
		 */
		Vec2 edgeLength{};

		/**
		 * \brief Box Origin Point
		 */
		Vec2 originPoint{};
		/**
		 * \brief Center To Bottom-Left Offset
		 */
		Vec2 offset{};

		/**
		 * \brief Rect Rotation
		 */
		float rotation{0};

		/**
		 * \brief Exported Vert [bottom-left, bottom-right, top-right, top-left], dynamic calculated
		 */
		Vec2 v0{};
		Vec2 v1{};
		Vec2 v2{};
		Vec2 v3{};


		/**
		 * \brief Normal Vector
		 */
		Vec2 normalU{};
		Vec2 normalV{};

		Shape::OrthoRectFloat maxOrthoBound{};

		void setSize(const float w, const float h) {
			edgeLength.set(w, h);
		}

		void update() {
			const float cos = Math::cosDeg(rotation);
			const float sin = Math::sinDeg(rotation);

			v0.set(offset).rotate(cos, sin);
			v1.set(edgeLength.x, 0).rotate(cos, sin);
			v3.set(0, edgeLength.y).rotate(cos, sin);
			v2 = v1 + v3;

			normalU = v3;
			normalV = v1;

			v0 += originPoint;
			v1 += v0;
			v2 += v0;
			v3 += v0;

			auto [xMin, xMax] = std::minmax({v0.x, v1.x, v2.x, v3.x});
			auto [yMin, yMax] = std::minmax({v0.y, v1.y, v2.y, v3.y});

			maxOrthoBound.setVert(xMin, yMin, xMax, yMax);
		}

		[[nodiscard]] bool axisOverlap(const RectBox& other, const Vec2& axis) const {
			float box1_min   = v0.dot(axis);
			float box1_max   = box1_min;
			float projection = v1.dot(axis);
			box1_min         = std::min(box1_min, projection);
			box1_max         = std::max(box1_max, projection);
			projection       = v2.dot(axis);
			box1_min         = std::min(box1_min, projection);
			box1_max         = std::max(box1_max, projection);
			projection       = v3.dot(axis);
			box1_min         = std::min(box1_min, projection);
			box1_max         = std::max(box1_max, projection);

			float other_min = other.v0.dot(axis);
			float other_max = other_min;
			projection      = other.v1.dot(axis);
			other_min       = std::min(other_min, projection);
			other_max       = std::max(other_max, projection);
			projection      = other.v2.dot(axis);
			other_min       = std::min(other_min, projection);
			other_max       = std::max(other_max, projection);
			projection      = other.v3.dot(axis);
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
			return VertGroup{v0, v1, v2, v3};
		}
	};
}
