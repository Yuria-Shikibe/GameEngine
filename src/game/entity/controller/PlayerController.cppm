//
// Created by Matrix on 2024/5/13.
//

export module Game.Entity.Controller.Player;

export import Game.Entity.Controller;
import Math;

export namespace Game{
	class RealityEntity;
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

			if(usingOrthoMove){
				if(!currentMoveCommands.isZero(0.005f)){
					moveCommand.expectedVelocity = currentMoveCommands;
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


			currentMoveCommands.setZero();
		}

		[[nodiscard]] bool selectable() const override{
			return false;
		}
	};
}
