module Graphic.Effect;

import Graphic.Effect.Manager;

Graphic::Effect* Graphic::EffectDrawer::suspendOn(::Graphic::EffectManager* manager) const{
	return manager->suspend()->setDrawer(this);
}
