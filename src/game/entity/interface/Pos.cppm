module;

export module Game.Entity.Pos;

export import Geom.Position;
export import Geom.Vector2D;

import Game.Entity;

export namespace Game {
	class PosedEntity : public Geom::Position2D{
	protected:
		Geom::Vec2 position{};

	public:
		~PosedEntity() override = default;

		[[nodiscard]] float getX() const override {
			return position.x;
		}

		[[nodiscard]] float getY() const override {
			return position.y;
		}

		void setX(const float x) override {
			this->position.x = x;
		}

		void setY(const float y) override {
			this->position.y = y;
		}
	};
}
