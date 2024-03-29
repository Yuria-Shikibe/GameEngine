//
// Created by Matrix on 2024/3/9.
//

export module Assets.Effects;

export import Graphic.Effect;

import Math.Interpolation;
import Math.Rand;

import Graphic.FxFunc;
import Graphic.Draw;
import std;

export namespace Assets::Effects{
	using namespace Graphic;
	namespace Interp = Math::Interp;

	std::unique_ptr<EffectDrawer>
		circleOut = makeEffect(60.0f, [](const Graphic::Effect& effect){
			Draw::color(Colors::WHITE, effect.color, effect.progress.getMargin(0.35f));
			Draw::Line::setLineStroke(effect.progress.getInv() * 4.5f);
			Draw::Line::circle(effect.trans.vec.x, effect.trans.vec.y, effect.progress.get(Interp::pow2Out) * 120.0f);
		}),

		squareOut = makeEffect(60.0f, [](const Graphic::Effect& effect){
			Draw::color(Colors::WHITE, effect.color, effect.progress.getMargin(0.35f));
			Draw::Line::setLineStroke(effect.progress.getInv() * 4.5f);
			Draw::Line::square(effect.trans.vec.x, effect.trans.vec.y, effect.progress.get(Interp::pow2Out) * 120.0f, 45);
		}),

		sparkLarge = makeEffect(60.0f, [](const Graphic::Effect& e){
			Draw::color(e.color, Colors::WHITE, e.progress.getInv() * 0.3f);
			Draw::Line::setLineStroke(e.progress.getInv() * 8.6f);

			Draw::setTexture(Draw::defaultLightTexture);
			Draw::setZ(e.zOffset);
			Graphic::splashVec(e.handle, {.count = 18, .progress = e.progress.get(Math::Interp::pow2Out) * 27.0f, .radius = {3, 15}},
				[vecSrc = e.trans.vec, progress = e.progress.getInv()](const auto vec2, auto& rand){
				Draw::Line::lineAngle<BatchWorld>(
					{vecSrc + vec2, vec2.angle()},
					progress * rand.random(12.0f, 33.0f) + 2.0f
				);
			});
			Draw::setTexture();
		});

	// EffectDrawer_Func<[](const Graphic::Effect& effect){
	// 	Draw::color(Colors::WHITE, effect.color, effect.progress.getMargin(0.35f));
	// 	Draw::Line::setLineStroke(effect.progress.getInv() * 4.5f);
	// 	Draw::Line::square(effect.position.x, effect.position.y, effect.progress.get(Interp::pow2Out) * 120.0f, 45);
	// }> squareOut;

	// EffectDrawer_Multi multi{circleOut.get()};
}
