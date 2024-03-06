module Game.Entity.Controller;

import Game.Entity.RealityEntity;

void Game::Controller::update() {
	updateTarget();

	moveCommand.updateCurrent(this->owner->position);
}
