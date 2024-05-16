module Game.Entity.Controller;

import Game.Entity.RealityEntity;

std::weak_ptr<Game::RealityEntity> Game::Controller::getOwnerWeakPtr() const{
	if(owner){
		return {std::static_pointer_cast<Game::RealityEntity>(owner->obtainSharedSelf())};
	}
	return std::weak_ptr<Game::RealityEntity>{};
}

Game::Controller::Controller(RealityEntity* const owner): owner(owner){
	owner->assignController();
}

void Game::Controller::assignTurretTarget(std::vector<Geom::Vec2>&& targets){
	turretTargets = std::move(targets);
	owner->targetUpdated();
}

void Game::Controller::update() {
	updateTarget();

	moveCommand.updateCurrent(this->owner->trans);
}
