module ;

export module Geom.InterSector;

import <algorithm>;
import Concepts;

import Geom.Vector2D;

import Geom.Shape;
import Geom.Shape.Circle;
import Geom.Shape.Rect_Orthogonal;

using namespace Geom::Shape;

export namespace Geom{
	Vec2 arrive(const float x, const float y, const float destX, const float destY, const Vec2& curVel, const float radius, const float tolerance, const float speed, const float accel) {
		 auto toTarget = Vec2{destX - x, destY - y};

	     const float distance = toTarget.length();

	     if (distance <= tolerance) return toTarget.setZero();
	     float targetSpeed = speed;
	     if (distance <= radius) targetSpeed *= distance / radius;

	     return toTarget.sub(curVel.x / accel, curVel.y / accel).limit(targetSpeed);
	 }


	Vec2 arrive(const Position2D& pos, const Position2D& target, const Vec2& curVel, const float radius, const float tolerance, const float speed, const float smoothTime) {
	     return arrive(pos.getX(), pos.getY(), target.getX(), target.getY(), curVel, radius, tolerance, speed, smoothTime);
	 }

	template <Concepts::Number T>
	bool overlap(const T x, const T y, const T radius, const Rect_Orthogonal<T>& rect) {
		T closestX = std::clamp<T>(x, rect.getSrcX(), rect.getEndX());
		T closestY = std::clamp<T>(y, rect.getSrcY(), rect.getEndY());

		T distanceX = x - closestX;
		T distanceY = y - closestY;
		T distanceSquared = distanceX * distanceX + distanceY * distanceY;

		return distanceSquared <= radius * radius;
	}

	template <Concepts::Number T>
	bool overlap(const Circle<T>& circle, const Rect_Orthogonal<T>& rect){
		return Geom::overlap<T>(circle.getCX(), circle.getCY(), circle.getRadius(), rect);
	}
}
