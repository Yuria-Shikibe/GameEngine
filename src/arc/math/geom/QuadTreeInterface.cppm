//
// Created by Matrix on 2024/5/31.
//

export module Geom.QuadTree.Interface;

export import Geom.Vector2D;
export import Geom.Rect_Orthogonal;
import ext.Concepts;

export namespace Geom{
	template <typename T, Concepts::Number N = float>
	struct QuadTreeAdaptable{
		Rect_Orthogonal<N> getBound() const noexcept = delete;

		bool roughIntersectWith(const T& other) const = delete;

		bool exactIntersectWith(const T& other) const = delete;

		bool containsPoint(typename Vector2D<N>::PassType point) const = delete;
	};
}
