export module Geom.Shape.Circle;

import <vector>;
import Geom.Vector2D;
import Concepts;
import Geom.Shape;


export namespace Geom::Shape{
	template <Concepts::Number T>
	class Circle final : virtual public Shape<T>{
		T cX{ 0 };
		T cY{ 0 };
		T radius{ 0 };

	public:
		~Circle() override = default;

		Circle() = default;

		Circle(const T cX, const T cY, const T radius)
			: cX(cX),
			  cY(cY),
			  radius(radius){
		}

		[[nodiscard]] T getCX() const{
			return cX;
		}

		void setCX(const T x){
			this->cX = x;
		}

		[[nodiscard]] T getCY() const{
			return cY;
		}

		void setCY(const T y){
			this->cY = y;
		}

		[[nodiscard]] T getRadius() const{
			return radius;
		}

		void setRadius(const T r){
			this->radius = r;
		}

		[[nodiscard]] Vector2D getCenter() const{
			return { cX, cY };
		}

		[[nodiscard]] bool contains(const T& other) const override{
			return true;
		}

		[[nodiscard]] bool inBound_edgeExclusive(const Vector2D& v) const override{
			return true;
		}
		[[nodiscard]] bool inBound_edgeInclusive(const Vector2D& v) const override{
			return true;
		}
		T maxDiagonalLen() const override{
			return radius * 2;
		}
		T maxDiagonalSqLen() const override{
			return radius * radius * 4;
		}
		[[nodiscard]] bool overlap(const T& other) const override{
			return true;
		}
		std::vector<Vector2D>& vertices(std::vector<Vector2D>& collector) const override{
			return collector;
		}

	};
}
