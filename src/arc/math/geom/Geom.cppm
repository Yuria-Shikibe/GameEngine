module ;

export module Geom;

import <algorithm>;
import <cmath>;
import <vector>;
import <execution>;
import Concepts;
import Math;

import Geom.Vector2D;

import Geom.Shape;
import Geom.Shape.Circle;
import Geom.Shape.RectBox;
import Geom.Shape.Rect_Orthogonal;


using namespace Geom::Shape;

export namespace Geom {
	constexpr Vec2 intersectCenterPoint(const RectBox_Brief& subject, const RectBox_Brief& object) {
		const float x0 = Math::max(subject.v0.x, object.v0.x);
		const float y0 = Math::max(subject.v0.y, object.v0.y);
		const float x1 = Math::min(subject.v1.x, object.v1.x);
		const float y1 = Math::min(subject.v1.y, object.v1.y);

		return { (x0 + x1) * 0.5f, (y0 + y1) * 0.5f };
	}

	float dstToLine(Vec2 vec2, const Vec2 pointOnLine, const Vec2 directionVec) {
		if(directionVec.isZero()) return vec2.dst(pointOnLine);
		vec2.sub(pointOnLine);
		const auto dot  = vec2.dot(directionVec);
		const auto porj = dot * dot / directionVec.length2();

		return std::sqrtf(vec2.length2() - porj);
	}

	float dstToLineSeg(const Vec2 vec2, const Vec2 p1, Vec2 p2) {
		p2 -= p1;
		return dstToLine(vec2, p1, p2);
	}

	Vec2 arrive(const float x, const float y, const float destX, const float destY, const Vec2 curVel,
	            const float radius, const float tolerance, const float speed, const float accel) {
		auto toTarget = Vec2{ destX - x, destY - y };

		const float distance = toTarget.length();

		if(distance <= tolerance) return toTarget.setZero();
		float targetSpeed = speed;
		if(distance <= radius) targetSpeed *= distance / radius;

		return toTarget.sub(curVel.x / accel, curVel.y / accel).limit(targetSpeed);
	}

	constexpr float continousTestScl = 1.0f;

	Vec2 intersectionLine(const Vec2 p1, const Vec2 p2, const Vec2 p3, const Vec2 p4) {
		const float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
		const float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;

		const float dx1 = x1 - x2;
		const float dy1 = y1 - y2;

		const float dx2 = x3 - x4;
		const float dy2 = y3 - y4;

		const float det = dx1 * dy2 - dy1 * dx2;

		if (det == 0.0f) {
			return Vec2{(x1 + x2) * 0.5f, (y1 + y2) * 0.5f};  // Return the midpoint of overlapping lines
		}

		const float pre = x1 * y2 - y1 * x2, post = x3 * y4 - y3 * x4;
		const float x   = (pre * dx2 - dx1 * post) / det;
		const float y   = (pre * dy2 - dy1 * post) / det;

		return Vec2{x, y};
	}

	 std::optional<Vec2> intersectSegments(const Vec2 p1, const Vec2 p2, const Vec2 p3, const Vec2 p4){
		const float x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y, x3 = p3.x, y3 = p3.y, x4 = p4.x, y4 = p4.y;

		const float dx1 = x2 - x1;
		const float dy1 = y2 - y1;

		const float dx2 = x4 - x3;
		const float dy2 = y4 - y3;

		const float d = dy2 * dx1 - dx2 * dy1;
		if(d == 0) return std::nullopt;

		const float yd = y1 - y3;
		const float xd = x1 - x3;
		const float ua = (dx2 * yd - dy2 * xd) / d;
		if(ua < 0 || ua > 1) return std::nullopt;

		const float ub = (dx1 * yd - dy1 * xd) / d;
		if(ub < 0 || ub > 1) return std::nullopt;

		return Vec2{x1 + dx1 * ua, y1 + dy1 * ua};
	}

	bool intersectSegments(const Vec2 p1, const Vec2 p2, const Vec2 p3, const Vec2 p4, Vec2& out){
		const float x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y, x3 = p3.x, y3 = p3.y, x4 = p4.x, y4 = p4.y;

		const float dx1 = x2 - x1;
		const float dy1 = y2 - y1;

		const float dx2 = x4 - x3;
		const float dy2 = y4 - y3;

		const float d = dy2 * dx1 - dx2 * dy1;
		if(d == 0) return false;

		const float yd = y1 - y3;
		const float xd = x1 - x3;
		const float ua = (dx2 * yd - dy2 * xd) / d;
		if(ua < 0 || ua > 1) return false;

		const float ub = (dx1 * yd - dy1 * xd) / d;
		if(ub < 0 || ub > 1) return false;

		out.set(x1 + dx1 * ua, y1 + dy1 * ua);
		return true;
	}

	Vec2 rectAvgIntersection(const RectBox_Brief& quad1, const RectBox_Brief& quad2) {
		Vec2 intersections{};
		float count = 0;

		for (int i = 0; i < 4; ++i) {
			for(int j = 0; j < 4; ++j) {
				if(Vec2 rst{}; intersectSegments(quad1[i], quad1[(i + 1) % 4], quad2[j], quad2[(j + 1) % 4], rst)) {
					count++;
					intersections += rst;
				}
			}
		}

		if(count > 0) {
			return intersections.div(count);
		}else {
			return intersections.set(quad1.v0).add(quad1.v2).add(quad2.v0).add(quad2.v2).scl(0.25f);
		}

	}

	void genContinousRectBox(std::vector<RectBox_Brief>& traces, Vec2 move, const RectBox& subject,
	                         const float scl = continousTestScl) {
		const float dst2  = move.length2();
		const float size2 = subject.projLen2(move);

		if(size2 < 0.025f)return;

		const size_t seg = Math::ceil(std::sqrtf(dst2 / size2) * scl + 0.00001f);

		move.div(static_cast<float>(seg));
		traces.assign(seg + 1, static_cast<RectBox_Brief>(subject));

		for(size_t i = 1; i <= seg; ++i) {
			traces[i].move(move, static_cast<float>(i));
		}
	}

	void genContinousRectBox(std::vector<RectBox_Brief>& traces, const Vec2 begin, Vec2 end, const RectBox& subject,
	                         const float scl = continousTestScl) {
		end -= begin;

		genContinousRectBox(traces, end, subject, scl);
	}

	void genContinousRectBox(std::vector<RectBox_Brief>& traces, Vec2 velo, const float delta, const RectBox& subject,
	                         const float scl = continousTestScl) {
		velo *= delta;

		genContinousRectBox(traces, velo, subject, scl);
	}

	OrthoRectFloat maxContinousBoundOf(const std::vector<RectBox_Brief>& traces) {
		const auto& front = traces.front().maxOrthoBound;
		const auto& back  = traces.back().maxOrthoBound;
		auto [minX, maxX] = std::minmax({ front.getSrcX(), front.getEndX(), back.getSrcX(), back.getEndX() });
		auto [minY, maxY] = std::minmax({ front.getSrcY(), front.getEndY(), back.getSrcY(), back.getEndY() });

		return OrthoRectFloat{ minX, minY, maxX - minX, maxY - minY };
	}


	Vec2 arrive(const Position2D& pos, const Position2D& target, const Vec2& curVel, const float radius,
	            const float tolerance, const float speed, const float smoothTime) {
		return arrive(pos.getX(), pos.getY(), target.getX(), target.getY(), curVel, radius, tolerance, speed,
		              smoothTime);
	}

	template <Concepts::Number T>
	bool overlap(const T x, const T y, const T radius, const Rect_Orthogonal<T>& rect) {
		T closestX = std::clamp<T>(x, rect.getSrcX(), rect.getEndX());
		T closestY = std::clamp<T>(y, rect.getSrcY(), rect.getEndY());

		T distanceX       = x - closestX;
		T distanceY       = y - closestY;
		T distanceSquared = distanceX * distanceX + distanceY * distanceY;

		return distanceSquared <= radius * radius;
	}

	template <Concepts::Number T>
	bool overlap(const Circle<T>& circle, const Rect_Orthogonal<T>& rect) {
		return Geom::overlap<T>(circle.getCX(), circle.getCY(), circle.getRadius(), rect);
	}
}
