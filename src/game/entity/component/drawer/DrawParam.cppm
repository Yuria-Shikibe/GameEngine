//
// Created by Matrix on 2024/3/15.
//

export module Game.Drawer.DrawParam;

export import Geom.Vector2D;

export namespace Game::Drawer{
	struct DrawParam {
		float progress{};
		float rotation{};
		Geom::Vec2 position{};
		float zLayer{};



	};
}
