//
// Created by Matrix on 2024/3/9.
//

export module Assets.Effects;

export import Graphic.Effect;

import Math.Interpolation;

import Graphic.Draw;
import Graphic.Draw.Lines;

export namespace Assets::Effects{
	using namespace Graphic;
	namespace Interp = Math::Interp;
	EffectDrawer_Func<[](const Graphic::Effect& effect){
		Draw::color(Colors::WHITE, effect.color, effect.progress.getMargin(0.35f));
		Draw::Line::setLineStroke(effect.progress.getInv() * 4.5f);
		Draw::Line::circle(effect.position.x, effect.position.y, effect.progress.get(Interp::pow2Out) * 120.0f);
	}> CircleDrawer;

	EffectDrawer_Func<[](const Graphic::Effect& effect){
		Draw::color(Colors::WHITE, effect.color, effect.progress.getMargin(0.35f));
		Draw::Line::setLineStroke(effect.progress.getInv() * 4.5f);
		Draw::Line::square(effect.position.x, effect.position.y, effect.progress.get(Interp::pow2Out) * 120.0f, 45);
	}> squareOut;
}
