//
// Created by Matrix on 2024/3/9.
//

export module Assets.Effects;

export import Graphic.Effect;

import Math.Interpolation;
import Math.Rand;

import Graphic.FxFunc;
import Graphic.Trail;
import Graphic.Draw;
import std;

export namespace Assets::Effects{
	/**
	 * @brief Using rotation as width
	 */
	struct EffectDrawer_TrailFade final : Graphic::EffectDrawer{
		[[nodiscard]] constexpr EffectDrawer_TrailFade() = default;

		[[nodiscard]] constexpr explicit EffectDrawer_TrailFade(float defaultLifetime)
			: EffectDrawer{defaultLifetime}{}

		struct Data{
			Graphic::Trail trail{};
		};

		void operator()(Graphic::Effect& effect) const override{
			const auto& [trail] = std::any_cast<Data&>(effect.additionalData);

			trail.each<Graphic::Trail::DefDraw, false>(effect.trans.rot, Graphic::Trail::DefDraw{effect.color},
			                                           trail.size() * effect.progress.get());
		}
	};

	/**
	 * @brief Using rotation as width
	 */
	struct EffectDrawer_TrailFade_WithLerp final : Graphic::EffectDrawer{
		[[nodiscard]] constexpr EffectDrawer_TrailFade_WithLerp() = default;

		[[nodiscard]] constexpr explicit EffectDrawer_TrailFade_WithLerp(float defaultLifetime)
			: EffectDrawer{defaultLifetime}{}

		struct Data{
			Graphic::Trail trail{};
			Graphic::Color endColor{};
		};

		void operator()(Graphic::Effect& effect) const override{
			const auto& [trail, endColor] = std::any_cast<Data&>(effect.additionalData);

			trail.each(effect.trans.rot, Graphic::Trail::DefDraw_WithLerp{effect.color, endColor},
			           trail.size() * effect.progress.get());
		}
	};

	Graphic::Effect* genTrailFade(Graphic::EffectManager* manager, Graphic::Trail&& points, const float width = 1.0f){
		static constexpr EffectDrawer_TrailFade drawer{60.0f};
		const auto effect = drawer.suspendOn(manager);
		effect->trans.rot = width;
		effect->trans.vec = points.getLastPos();
		effect->additionalData = decltype(drawer)::Data{std::move(points)};
		return effect;
	}

	Graphic::Effect* genTrailFade(Graphic::EffectManager* manager, Graphic::Trail&& points, const float width = 1.0f,
	                              const Graphic::Color& endColor = Graphic::Colors::GRAY){
		static constexpr EffectDrawer_TrailFade_WithLerp drawer{60.0f};
		const auto effect = drawer.suspendOn(manager);
		effect->trans.rot = width;
		effect->trans.vec = points.getLastPos();
		effect->additionalData = decltype(drawer)::Data{std::move(points), endColor};
		return effect;
	}

	std::unique_ptr<Graphic::EffectDrawer>
		circleOut = Graphic::makeEffect(60.0f, [](const Graphic::Effect& effect){
			using namespace Graphic;

			Draw::color(Colors::WHITE, effect.color, effect.progress.getMargin(0.35f));
			Draw::Line::setLineStroke(effect.progress.getInv() * 4.5f);
			Draw::Line::circle(effect.trans.vec.x, effect.trans.vec.y,
			                            effect.progress.get(Math::Interp::pow2Out) * 120.0f);
		}),

		squareOut = Graphic::makeEffect(60.0f, [](const Graphic::Effect& effect){
			using namespace Graphic;
			
			Draw::color(Colors::WHITE, effect.color, effect.progress.getMargin(0.35f));
			Draw::Line::setLineStroke(effect.progress.getInv() * 4.5f);
			Draw::Line::square(effect.trans.vec.x, effect.trans.vec.y,
			                            effect.progress.get(Math::Interp::pow2Out) * 120.0f, 45);
		}),

		sparkLarge = Graphic::makeEffect(60.0f, [](const Graphic::Effect& e){
			using namespace Graphic;

			Draw::color(e.color, Colors::WHITE, e.progress.getInv() * 0.3f);
			Draw::Line::setLineStroke(e.progress.getInv() * 8.6f);

			Draw::setTexture(Draw::globalState.defaultLightTexture);
			Draw::setZ(e.zOffset);
			splashVec(e.handle, {
				                   .count = 18, .progress = e.progress.get(Math::Interp::pow2Out) * 27.0f,
				                   .radius = {3, 15}
			                   },
			                   [vecSrc = e.trans.vec, progress = e.progress.getInv()](const auto vec2, auto& rand){
				                   Draw::Line::lineAngle<BatchWorld>(
					                   {vecSrc + vec2, vec2.angle()},
					                   progress * rand.random(12.0f, 33.0f) + 2.0f
				                   );
			                   });
			Draw::setTexture();
		}),

		explode = Graphic::makeEffect(120.0f, [](const Graphic::Effect& e){
			using Math::Interp::operator|;
			using namespace Graphic;
			Draw::setTexture(Draw::globalState.defaultLightTexture);

			Draw::color(e.color, Colors::WHITE, e.progress.getInv() * 0.3f);
			Draw::setZ(e.zOffset);
			
			Draw::Line::setLineStroke(e.progress.getInv() * 7.f);
			Draw::Line::circle<BatchWorld>(e.trans.vec.x, e.trans.vec.y,
			                            e.progress.get(Math::Interp::pow3Out) * e.trans.rot * 1.4f);

			splashVec(e.handle, {
				                   .count = 18, .progress = e.progress.get(Math::Interp::pow2Out),
				                   .radius = {e.trans.rot * 0.3f, e.trans.rot * 1.25f}
			                   },
           [&e](const Geom::Vec2 vec2, Math::Rand& rand){
	           const auto pos = e.trans.vec + vec2;
           		Draw::color(Colors::DARK_GRAY, Colors::GRAY, rand.random(0.1f, 0.5f));
               Draw::Fill::circle<BatchWorld>(pos.x, pos.y, (e.progress.getInv() | Math::Interp::pow2In) * rand.random(e.trans.rot * 0.2f, e.trans.rot * 0.6f) + 2.0f);
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
