module Game.Entity.Turret;

import Graphic.Draw;

void Game::TurretTrait::drawDebug(const TurretEntity* turret) const{
	Graphic::Draw::Line::line(turret->trans.vec, turret->getTargetPos(), Graphic::Colors::PALE_GREEN.copy().setA(0.5f), Graphic::Colors::LIGHT_GRAY.copy().setA(0.5f));
}