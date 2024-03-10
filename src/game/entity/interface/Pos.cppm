module;

export module Game.Entity.PosedEntity;

export import Geom.Position;
export import Geom.Vector2D;

export namespace Game {
	class PosedEntity : public Geom::Position2D{
	public:
		Geom::Vec2 position{};
		float rotation{0};

		float layer{0};

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

		[[nodiscard]] virtual Geom::Vec2& getPos() {
			return position;
		}

		[[nodiscard]] virtual Geom::Vec2 copyPos() const {
			return position;
		}
	};
}
