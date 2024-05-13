module Game.Entity.Controller.Player;

import Game.Core;

Game::PlayerController::PlayerController(Game::RealityEntity* owner): Game::Controller{owner}{
	Game::core->reBindPlayerController(this);
	moveCommand.overrideControl = true;
}

Game::PlayerController::~PlayerController(){
	Game::core->reBindPlayerController(nullptr);
}


