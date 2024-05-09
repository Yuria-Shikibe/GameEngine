module;

export module Game.Entity.Healthed;

import Game.Entity;
import std;

import Game.Settings.DamageTrait;

export namespace Game {
	class Healthed{
	protected: //TODO atomic maybe
		float health{0};
		float healthMaximum{100};

		//TODO should these thing be here??
		float empResistance{0};
		float empMaximumResistance{0};
	public:
		virtual ~Healthed() = default;

		virtual void dealDamage(const DamageComposition& val) {
			health -= val.sum();
		}

		[[nodiscard]] virtual float getHealth() const noexcept{
			return health;
		}

		virtual void setHealth(const float health) noexcept{
			this->health = health;
		}

		virtual void dealMend(const float val) noexcept{
			health += val;
		}

		[[nodiscard]] virtual bool invalid() const noexcept{
			return health < 0;
		}

		[[nodiscard]] virtual float getHealthRatio() const noexcept{
			return health / healthMaximum;
		}

		virtual void kill() {
			health = std::numeric_limits<float>::lowest();
		}
	};
}
