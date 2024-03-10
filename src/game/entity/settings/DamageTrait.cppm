//
// Created by Matrix on 2024/3/10.
//

export module Game.Settings.DamageTrait;

export namespace Game{
	struct DamageTrait {
		float fullDamage{0.0f};
		float pierceForce{0.0f};

		float splashRadius{-1.0f};
		float splashAngle{360.0f};

		[[nodiscard]] bool isSplashes() const{
			return splashRadius > 0;
		}

		[[nodiscard]] bool heal() const{
			return fullDamage < 0;
		}
	};

	struct DamageComposition {
		DamageTrait materialDamage{}; //Maybe basic damage
		DamageTrait fieldDamage{}; //Maybe real damage

		[[nodiscard]] float sum() const {
			return materialDamage.fullDamage + fieldDamage.fullDamage;
		}
	};
}
