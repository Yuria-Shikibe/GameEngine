module Game.Entity.Controller;

import Game.Entity.RealityEntity;

Game::Controller::Controller(const RealityEntity* const owner): owner(owner){
	owner->assignController();
}

void Game::Controller::update() {
	updateTarget();

	moveCommand.updateCurrent(this->owner->position);
}
