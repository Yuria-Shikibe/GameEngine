module;

export module Geom.Shape.RectBox;

import Geom.Vector2D;
import Geom.Shape.Rect_Orthogonal;

import Math;
import std;

export namespace Geom {
	/**
	 * \brief Mainly Used For Continous Collidsion Test
	 * @code
	 * v3 +-----+ v2
	 *    |     |
	 *    |     |
	 * v0 +-----+ v1
	 * @endcode
	 */
	struct QuadBox {
		using VertGroup = std::array<Vec2, 4>;

		Vec2 v0{};
		Vec2 v1{};
		Vec2 v2{};
		Vec2 v3{};

		/**
		 * \brief Exported Vert [bottom-left, bottom-right, top-right, top-left], dynamic calculated
		 */
		Shape::OrthoRectFloat maxOrthoBound{};

		[[nodiscard]] constexpr Vec2 operator[](const int i) const {
			switch(i) {
				case 0 : return v0;
				case 1 : return v1;
				case 2 : return v2;
				case 3 : return v3;
				default: return Geom::QNAN2;
			}
		}

		constexpr void move(const Vec2 trans, const QuadBox& other) {
			v0.set(other.v0).add(trans);
			v1.set(other.v1).add(trans);
			v2.set(other.v2).add(trans);
			v3.set(other.v3).add(trans);

			maxOrthoBound = other.maxOrthoBound;
			maxOrthoBound.move(trans.x, trans.y);
		}

		constexpr void move(const Vec2 vec2) {
			v0.add(vec2);
			v1.add(vec2);
			v2.add(vec2);
			v3.add(vec2);

			maxOrthoBound.move(vec2.x, vec2.y);
		}

		constexpr void move(const Vec2 vec2, const float scl) {
			v0.add(vec2, scl);
			v1.add(vec2, scl);
			v2.add(vec2, scl);
			v3.add(vec2, scl);

			maxOrthoBound.move(vec2.x * scl, vec2.y * scl);
		}

		constexpr void updateBound(){
			auto [xMin, xMax] = std::minmax({v0.x, v1.x, v2.x, v3.x});
			auto [yMin, yMax] = std::minmax({v0.y, v1.y, v2.y, v3.y});

			maxOrthoBound.setVert(xMin, yMin, xMax, yMax);
		}

		[[nodiscard]] constexpr bool axisOverlap(const QuadBox& other, const Vec2& axis) const {
			float box1_min   = v0.dot(axis);
			float box1_max   = box1_min;
			float projection = v1.dot(axis);
			box1_min         = Math::min(box1_min, projection);
			box1_max         = Math::max(box1_max, projection);
			projection       = v2.dot(axis);
			box1_min         = Math::min(box1_min, projection);
			box1_max         = Math::max(box1_max, projection);
			projection       = v3.dot(axis);
			box1_min         = Math::min(box1_min, projection);
			box1_max         = Math::max(box1_max, projection);

			float other_min = other.v0.dot(axis);
			float other_max = other_min;
			projection      = other.v1.dot(axis);
			other_min       = Math::min(other_min, projection);
			other_max       = Math::max(other_max, projection);
			projection      = other.v2.dot(axis);
			other_min       = Math::min(other_min, projection);
			other_max       = Math::max(other_max, projection);
			projection      = other.v3.dot(axis);
			other_min       = Math::min(other_min, projection);
			other_max       = Math::max(other_max, projection);

			return box1_max >= other_min && other_max >= box1_min;
		}

		[[nodiscard]] constexpr bool overlapExact(const QuadBox& other,
			const Vec2 axis_1, const Vec2 axis_2,
			const Vec2 axis_3, const Vec2 axis_4
		) const {
			return
				axisOverlap(other, axis_1) && axisOverlap(other, axis_2) &&
				axisOverlap(other, axis_3) && axisOverlap(other, axis_4);
		}

		[[nodiscard]] constexpr bool overlapRough(const QuadBox& other) const {
			return
				maxOrthoBound.overlap(other.maxOrthoBound);
		}

		[[nodiscard]] constexpr bool contains(const Geom::Vec2 point) const {
			bool oddNodes = false;

			for(int i = 0; i < 4; ++i){
				const Vec2 vertice     = this->operator[](i);
				const Vec2 lastVertice = this->operator[]((i + 1) % 4);
				if((vertice.y < point.y && lastVertice.y >= point.y) || (lastVertice.y < point.y && vertice.y >= point.y)){
					if(vertice.x + (point.y - vertice.y) * (lastVertice.x - vertice.x) / (lastVertice.y - vertice.y) < point.x){
						oddNodes = !oddNodes;
					}
				}
			}
			return oddNodes;
		}

		[[nodiscard]] constexpr VertGroup verts() const {
			return VertGroup{v0, v1, v2, v3};
		}

		[[nodiscard]] constexpr Vec2 getNormalVec(const int index) const {
			const auto begin = this->operator[](index);
			const auto end   = this->operator[]((index + 1) % 4);

			return (begin - end).rotateRT();
		}
	};

	struct RectBoxBrief{
		/**
		 * \brief x for rect width, y for rect height, static
		 */
		Vec2 sizeVec2{};

		/**
		 * \brief Center To Bottom-Left Offset
		 */
		Vec2 offset{};
	};

	struct RectBox : QuadBox, RectBoxBrief{
		//Transient Fields

		/**
		 * \brief Box Origin Point
		 * Should Be Mass Center if possible!
		 */
		Vec2 originPoint{};
		/**
		 * \brief Rect Rotation
		 */
		float rotation{0};

		/**
		 * \brief
		 * Normal Vector for v0-v1, v2-v3
		 * Edge Vector for v1-v2, v3-v0
		 */
		Vec2 normalU{};

		/**
		 * \brief
		 * Normal Vector for v1-v2, v3-v0
		 * Edge Vector for v0-v1, v2-v3
		 */
		Vec2 normalV{};

		using QuadBox::v0;
		using QuadBox::v1;
		using QuadBox::v2;
		using QuadBox::v3;
		using QuadBox::maxOrthoBound;
		using QuadBox::axisOverlap;
		using QuadBox::move;
		using QuadBox::overlapRough;
		using QuadBox::overlapExact;

		constexpr RectBox& copyNecessary(const RectBox& other){
			rotation = other.rotation;
			originPoint = other.originPoint;
			sizeVec2 = other.sizeVec2;
			offset = other.offset;

			return *this;
		}

		constexpr void setSize(const float w, const float h) {
			sizeVec2.set(w, h);
		}

		[[nodiscard]] constexpr Vec2 getNormalVec(const int index) const {
			switch(index) {
				case 0 : return -normalU;
				case 1 : return normalV;
				case 2 : return normalU;
				case 3 : return -normalV;
				default: return Geom::QNAN2;
			}
		}

		/**
		 * \param mass
		 * \param scale Manually assign a correction scale
		 * \param lengthRadiusRatio to decide the R(radius) scale for simple calculation
		 * \brief From: [mr^2/4 + ml^2 / 12]
		 * \return Rotational Inertia Estimation
		 */
		[[nodiscard]] constexpr float getRotationalInertia(const float mass, const float scale = 1 / 12.0f, const float lengthRadiusRatio = 0.25f) const {
			return sizeVec2.length2() * (scale + lengthRadiusRatio) * mass;
		}

		constexpr void update(const Vec2 pos, const float rot) {
			originPoint = pos;
			rotation = rot;

			const float cos = Math::cosDeg(rotation);
			const float sin = Math::sinDeg(rotation);

			v0.set(offset).rotate(cos, sin);
			v1.set(sizeVec2.x, 0).rotate(cos, sin);
			v3.set(0, sizeVec2.y).rotate(cos, sin);
			v2 = v1 + v3;

			normalU = v3;
			normalV = v1;

			v0 += originPoint;
			v1 += v0;
			v2 += v0;
			v3 += v0;

			updateBound();
		}

		[[nodiscard]] constexpr bool overlapExact(const RectBox& other) const {
			return
				axisOverlap(other, normalU) && axisOverlap(other, normalV) &&
				axisOverlap(other, other.normalU) && axisOverlap(other, other.normalV);
		}

		[[nodiscard]] constexpr bool overlapExact(const QuadBox& other, const Vec2 normalU, const Vec2 normalV) const {
			return
				axisOverlap(other, this->normalU) && axisOverlap(other, this->normalV) &&
				axisOverlap(other, normalU) && axisOverlap(other, normalV);
		}

		[[nodiscard]] constexpr float projLen2(const Vec2 axis) const {
			const Vec2 diagonal = v2 - v0;
			const float dot = diagonal.dot(axis);
			return dot * dot / axis.length2();
		}

		[[nodiscard]] float projLen(const Vec2 axis) const {
			return std::sqrt(projLen2(axis));
		}

		[[nodiscard]] constexpr bool contains(const Vec2 point) const {
			bool oddNodes = false;

			for(int i = 0; i < 4; ++i){
				const Vec2 vertice     = this->operator[](i);
				const Vec2 lastVertice = this->operator[]((i + 1) % 4);
				const Vec2 curEdge = i & 1 ? normalU : normalV;
				if((vertice.y < point.y && lastVertice.y >= point.y) || (lastVertice.y < point.y && vertice.y >= point.y)){
					if(vertice.x + (point.y - vertice.y) / curEdge.y * curEdge.x < point.x){
						oddNodes = !oddNodes;
					}
				}
			}
			return oddNodes;
		}
	};


}
