//
// Created by Matrix on 2024/5/13.
//

export module Game.Entity.Controller.Player;

import Game.Entity.RealityEntity;
export import Game.Entity.Controller;
import Math;
import Core;

export namespace Game{
	class PlayerController : public Controller{
		static constexpr float QuiteBig = 10000;

		Geom::Vec2 currentMoveCommands{};
		bool usingOrthoMove = false;

	public:
		[[nodiscard]] explicit PlayerController(::Game::RealityEntity* owner);

		~PlayerController() override;

		void setMoveDirection(const Geom::Vec2 dir){
			currentMoveCommands += dir * QuiteBig;
		}

		void update() override{
			Controller::update();

			currentMoveCommands = currentMoveCommands.sign() * QuiteBig;

			if(moveCommand.translatory){
				if(!currentMoveCommands.isZero(0.005f)){
					moveCommand.expectedVelocity = currentMoveCommands.rotate(moveCommand.curTrans.rot - 90);
					moveCommand.activateMove();
					moveCommand.activateRotate();
				} else{
					moveCommand.expectedVelocity.setZero();
					moveCommand.deactivateMove();
					moveCommand.deactivateRotate();
				}
			} else{
				//Rotate
				if(const int sign = Math::sign(currentMoveCommands.x)){
					moveCommand.activateRotate();
					moveCommand.expectedFaceAngle = moveCommand.curTrans.rot - 90 * sign;
				} else{
					moveCommand.deactivateRotate();
					moveCommand.expectedFaceAngle = moveCommand.curTrans.rot;
				}

				//Move
				if(const int sign = Math::sign(currentMoveCommands.y)){
					moveCommand.activateMove();
					moveCommand.expectedVelocity = Geom::Vec2{}.setPolar(moveCommand.curTrans.rot, QuiteBig * sign);
				} else{
					moveCommand.deactivateMove();
					moveCommand.expectedVelocity.setZero();
				}
			}

			turretTargets.clear();
			turretTargets.push_back(Core::Util::getMouseToWorld());
			getOwner()->targetUpdated();

			currentMoveCommands.setZero();
		}

		[[nodiscard]] bool selectable() const override{
			return false;
		}
	};
}
