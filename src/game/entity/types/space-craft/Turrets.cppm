//
// Created by Matrix on 2024/1/4.
//

export module Game.Entity.Turrets;

export import Game.Entity.RealityEntity;

import Geom.Vector2D;

export namespace Game{
	class TurretEntity;

	struct TurretTrait {
		virtual ~TurretTrait() = default;
		/**
		 * @brief In Degree
		 */
		float maxRotateSpeed{5};
		float maxRotateAccel{1.25f};

		float aimAngleTolerance{20};

		float initialDirection{0};
		float rotateLimitation{-1};

		float totalReload{};

		[[nodiscard]] bool hasRotateLimitation() const {
			return rotateLimitation > 0 && rotateLimitation < 360;
		}

		virtual void update(TurretEntity* turret) const = 0;

		virtual void draw(TurretEntity* turret) const = 0;

		virtual void shoot(TurretEntity* turret) const = 0;

	};

	class TurretEntity : public BasicRealityEntity {
	protected:
		const TurretTrait* trait{nullptr};
		Geom::Vec2 targetPosition{};

		float rotateSpeed{0};

		float reload{0};

		bool firing{false};

	};
}
