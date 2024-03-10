module Graphic.Effect;

import Graphic.Effect.Manager;

Graphic::Effect* Graphic::EffectDrawer::suspendOn(::Graphic::EffectManager* manager){
	return manager->suspend()->setDrawer(this);
}