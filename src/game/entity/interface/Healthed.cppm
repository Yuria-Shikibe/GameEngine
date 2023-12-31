module;

export module Game.Entity.Healthed;

import Game.Entity;
import <limits>;

export namespace Game {
	class Healthed{
	protected:
		float health{0};
	public:
		virtual ~Healthed() = default;

		virtual void dealDamage(const float val) {
			health -= val;
		}

		[[nodiscard]] virtual float getHealth() const {
			return health;
		}

		virtual void setHealth(const float health) {
			this->health = health;
		}

		virtual void dealMend(const float val) {
			health += val;
		}

		virtual bool invalid() {
			return health < 0;
		}

		virtual void kill() {
			health = std::numeric_limits<float>::lowest();
		}
	};
}
