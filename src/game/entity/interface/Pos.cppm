module;

export module Game.Entity.PosedEntity;

export import Geom.Position;
export import Geom.Vector2D;
export import Geom.Transform;

export namespace Game {
	class PosedEntity : public Geom::Position2D{
	public:
		Geom::Transform trans{};
		Geom::Transform vel{};
		Geom::Transform accel{};

		float zLayer{30};

		~PosedEntity() override = default;

		[[nodiscard]] float getX() const override {
			return trans.vec.x;
		}

		[[nodiscard]] float getY() const override {
			return trans.vec.y;
		}

		void setX(const float x) override {
			this->trans.vec.x = x;
		}

		void setY(const float y) override {
			this->trans.vec.y = y;
		}

		[[nodiscard]] Geom::Vec2& getPosRef() {
			return trans.vec;
		}

		[[nodiscard]] Geom::Vec2 getPos() const {
			return trans.vec;
		}
	};
}
