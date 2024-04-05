module ;

export module Geom;

import std;
import Concepts;
import Math;
import Math.Interpolation;

import Geom.Vector2D;

import Geom.Shape;
import Geom.Shape.Circle;
import Geom.Shape.RectBox;
import Geom.Rect_Orthogonal;


using namespace Geom::Shape;

export namespace Geom {
	constexpr Vec2 intersectCenterPoint(const QuadBox& subject, const QuadBox& object) {
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

	float dst2ToLine(Vec2 from, const Vec2 pointOnLine, const Vec2 directionVec) {
		if(directionVec.isZero()) return from.dst2(pointOnLine);
		from.sub(pointOnLine);
		const auto dot  = from.dot(directionVec);
		const auto porj = dot * dot / directionVec.length2();

		return from.length2() - porj;
	}

	float dst2ToLineSeg(const Vec2 vec2, const Vec2 vert1, Vec2 vert2) {
		vert2 -= vert1;
		return dstToLine(vec2, vert1, vert2);
	}

	float dstToSegment(const Vec2 p, const Vec2 a, const Vec2 b) {
		const float lenAB = a.dst(b);
		if (lenAB < Math::FLOAT_ROUNDING_ERROR) {
			return a.dst(p);
		}

		// AB dot AB
		if (const float dot = (p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y); dot < 0) {
			return a.dst(p);
		} else if (dot > lenAB * lenAB) {
			return a.dst(p);
		} else {
			const float t = dot / (lenAB * lenAB);
			return p.dst(a.x + t * (b.x - a.x), a.y + t * (b.y - a.y));
		}
	}

	float dst2ToSegment(const Vec2 p, const Vec2 a, const Vec2 b) {
		const float lenAB = a.dst(b);
		if (lenAB < Math::FLOAT_ROUNDING_ERROR) {
			return a.dst2(p);
		}

		// AB dot AB
		if (const float dot = (p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y); dot < 0) {
			return a.dst2(p);
		} else if (dot > lenAB * lenAB) {
			return a.dst2(p);
		} else {
			const float t = dot / (lenAB * lenAB);
			return p.dst2(a.x + t * (b.x - a.x), a.y + t * (b.y - a.y));
		}
	}

	Vec2 arrive(const Vec2 position, const Vec2 dest, const Vec2 curVel, const float smooth, const float radius, const float tolerance) {
		auto toTarget = Vec2{ dest - position };

		const float distance = toTarget.length();

		if(distance <= tolerance) return toTarget.setZero();
		float targetSpeed = curVel.length();
		if(distance <= radius) targetSpeed *= distance / radius;

		return toTarget.sub(curVel.x / smooth, curVel.y / smooth).limit(targetSpeed);
	}

	constexpr Vec2 intersectionLine(const Vec2 p1, const Vec2 p2, const Vec2 p3, const Vec2 p4) {
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

	template <Concepts::Derived<QuadBox> T>
	[[nodiscard]] constexpr Vec2 nearestEdgeNormal(const Vec2 p, const T& rectangle) {
		float minDistance = std::numeric_limits<float>::max();
		Vec2 closestEdgeNormal{};

		for (int i = 0; i < 4; i++) {
			auto a = rectangle[i];
			auto b = rectangle[(i + 1) % 4];

			const float d = ::Geom::dst2ToSegment(p, a, b);

			if (d < minDistance) {
				minDistance = d;
				closestEdgeNormal = rectangle.getNormalVec(i);
			}
		}

		return closestEdgeNormal;
	}

	template <Concepts::Derived<QuadBox> T>
	[[nodiscard]] constexpr Vec2 avgEdgeNormal(const Vec2 p, const T& rectangle) {
		Vec2 closestEdgeNormal{};

		std::array<std::pair<float, Vec2>, 4> normals{};

		for (int i = 0; i < 4; i++) {
			auto a = rectangle[i];
			auto b = rectangle[(i + 1) % 4];

			normals[i].first = ::Geom::dstToSegment(p, a, b) * a.dst(b);
			normals[i].second = rectangle.getNormalVec(i).normalize();
		}

		const float total = (normals[0].first + normals[1].first + normals[2].first + normals[3].first);

		for(auto& [weight, normal] : normals) {
			closestEdgeNormal.add(normal * Math::powIntegral<15>(weight / total));
		}

		// Yes this is cool
		// closestEdgeNormal /= std::accumulate(normals | std::ranges::views::elements<0> | std::ranges::common_range);

		return closestEdgeNormal;
	}

	Vec2 rectAvgIntersection(const QuadBox& quad1, const QuadBox& quad2) {
		Vec2 intersections{};
		float count = 0;

		Vec2 rst{};

		for(int i = 0; i < 4; ++i) {
			for(int j = 0; j < 4; ++j) {
				if(intersectSegments(quad1[i], quad1[(i + 1) % 4], quad2[j], quad2[(j + 1) % 4], rst)) {
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


	OrthoRectFloat maxContinousBoundOf(const std::vector<QuadBox>& traces) {
		const auto& front = traces.front().maxOrthoBound;
		const auto& back  = traces.back().maxOrthoBound;
		auto [minX, maxX] = std::minmax({ front.getSrcX(), front.getEndX(), back.getSrcX(), back.getEndX() });
		auto [minY, maxY] = std::minmax({ front.getSrcY(), front.getEndY(), back.getSrcY(), back.getEndY() });

		return OrthoRectFloat{ minX, minY, maxX - minX, maxY - minY };
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
