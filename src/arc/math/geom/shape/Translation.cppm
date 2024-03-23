//
// Created by Matrix on 2024/3/21.
//

export module Geom.Translation;

export import Geom.Vector2D;

export namespace Geom{
	struct Translation {
		Vec2 pos{};
		float rot{};
	};
}
