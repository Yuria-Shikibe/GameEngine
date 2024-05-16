export module Game.Entity.Controller.AI;

export import Game.Entity.Controller;
import Game.Core;
import Game.Entity.RealityEntity;

export namespace Game{
	class AiController : public Controller{
	public:
		[[nodiscard]] explicit AiController(RealityEntity* owner)
			: Controller{owner}{}

		void findTarget() override{
			turretTargets.clear();
			if(Game::core->playerController){
				if(Game::core->playerController->moveCommand.curTrans.vec.dst(owner->trans.vec) < 4000){
					shoot = true;
					turretTargets.push_back(Game::core->playerController->moveCommand.curTrans.vec);
				}else{
					shoot = false;
				}
			}
		}

		void update() override{
			Controller::update();

			moveCommand.assignTarget(moveCommand.curTrans.vec + Geom::Vec2{100, 0});

			owner->targetUpdated();
		}
	};
}