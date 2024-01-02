module;

export module Game.Entity.Pos;

export import Geom.Position;

export namespace Game {
	class Pos : public Geom::Position {
	protected:
		float x{};
		float y{};

	public:
		~Pos() override;

		[[nodiscard]] float getX() const override {
			return x;
		}

		[[nodiscard]] float getY() const override {
			return y;
		}

		void setX(float x) override {
			this->x = x;
		}

		void setY(float y) override {
			this->y = y;
		}
	};
}
