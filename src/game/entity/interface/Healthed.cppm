module;

export module Game.Entity.Healthed;

import Game.Entity;
import std;

import Game.Settings.DamageTrait;

export namespace Game {
	class Healthed{
	protected:
		float health{0};
		float healthMaximum{100};

		float empResistance{0};
		float empMaximumResistance{0};
	public:
		virtual ~Healthed() = default;

		virtual void dealDamage(const DamageComposition& val) {
			health -= val.sum();
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

		[[nodiscard]] virtual bool invalid() const {
			return health < 0;
		}

		[[nodiscard]] virtual float getHealthRatio() const{
			return health / healthMaximum;
		}

		virtual void kill() {
			health = std::numeric_limits<float>::lowest();
		}
	};
}
