//
// Created by Matrix on 2024/6/14.
//

export module Geom.Analytic;

export import Geom.Transform;
export import Geom.Vector2D;

export namespace Geom{
	Vec2 projTo(const Geom::Vec2 point, const Transform line){
		const auto dst = point - line.vec;
		const auto dir = dirNor(line.rot);
		return dir * dir.dot(dst) - dst;
	}

	Transform mirrowBy(const Transform& src, const Transform& axis){
		const auto dst = projTo(src.vec, axis);

		return Transform{src.vec + dst * 2, src.rot - 2 * (src.rot - axis.rot)};
	}
}
