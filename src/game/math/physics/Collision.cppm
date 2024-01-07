module;

export module Game.Math.Physics.Collision;

import Geom.Shape.RectBox;
import Geom.Vector2D;

namespace Game {
	class RealityEntity;
}

using Geom::Vec2;

export namespace Math::Phy {
	void calCollideTo(Game::RealityEntity* subject, const Game::RealityEntity* object, Vec2 intersection);
}
