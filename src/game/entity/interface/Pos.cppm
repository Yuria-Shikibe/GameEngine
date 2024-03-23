module;

export module Game.Entity.PosedEntity;

export import Geom.Position;
export import Geom.Vector2D;
export import Geom.Translation;

export namespace Game {
	class PosedEntity : public Geom::Position2D{
	public:
		Geom::Translation trans{};

		float layer{0};

		~PosedEntity() override = default;

		[[nodiscard]] float getX() const override {
			return trans.pos.x;
		}

		[[nodiscard]] float getY() const override {
			return trans.pos.y;
		}

		void setX(const float x) override {
			this->trans.pos.x = x;
		}

		void setY(const float y) override {
			this->trans.pos.y = y;
		}

		[[nodiscard]] virtual Geom::Vec2& getPos() {
			return trans.pos;
		}

		[[nodiscard]] virtual Geom::Vec2 copyPos() const {
			return trans.pos;
		}
	};
}
