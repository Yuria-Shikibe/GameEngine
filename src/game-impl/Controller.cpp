module Game.Entity.Controller;

import Game.Entity.RealityEntity;

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
