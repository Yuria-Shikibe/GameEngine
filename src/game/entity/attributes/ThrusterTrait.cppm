//
// Created by Matrix on 2024/5/9.
//

export module Game.Attributes.ThrusterTrait;

import Geom.Transform;
import Math;
import Math.Interpolation;
import std;

namespace Game{
	class SpaceCraft;

	export struct ThrusterDrawer{
		Geom::Transform pos{};
		virtual ~ThrusterDrawer() = default;

		virtual void draw(const SpaceCraft* baseEntity, float warmup) const = 0;
	};

	export struct ThrusterTrait{
		float forwardPowerScale{1.0f};
		float sidePowerScale{0.35f};
		float backwardPowerScale{0.105f};

		float getPowerScale(float yawAngle) const noexcept{
			yawAngle = Math::abs(Math::Angle::getAngleInPi(yawAngle));
			return Math::spanLerp(std::array{forwardPowerScale, sidePowerScale, backwardPowerScale}, yawAngle / Math::DEG_HALF);
		}

		std::vector<std::unique_ptr<ThrusterDrawer>> thursters{};

		void draw(const SpaceCraft* baseEntity, const float warmup) const{
			for(auto& thruster : thursters){
				thruster->draw(baseEntity, warmup);
			}
		}
	};
}
