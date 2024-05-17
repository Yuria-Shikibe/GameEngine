//
// Created by Matrix on 2024/3/10.
//

export module Game.Content.Type.BasicBulletType;

import Assets.Effects;

import Geom.Vector2D;
import Game.Entity.Bullet;
import Game.Core;
import Graphic.Draw;
import Graphic.Trail;

import Game.Graphic.Draw;
import ext.Guard;

export import Game.Delay;

export namespace Game::Content{
	struct BasicBulletType : BulletTrait {
		Graphic::Color effectColor = Graphic::Colors::AQUA_SKY;

		Graphic::Color trailBegin = Graphic::Colors::AQUA_SKY;
		Graphic::Color trailEnd = Graphic::Colors::GRAY;
		float trailWidth = 4.0f;

		[[nodiscard]] BasicBulletType(){
			maximumLifetime = 20.0f;
			initDamage.materialDamage.fullDamage = 100.0f;
		}

		void update(Bullet& bullet) const override{

		}

		float getMaximumRange() override{
			return maximumLifetime * initSpeed;
		}

		void onShoot(Bullet& bullet) const override{
			BulletTrait::onShoot(bullet);
			Assets::Effects::circleOut->suspendOn(Game::core->effectManager.get())->
				set({.vec = bullet.trans.vec}, effectColor);
		}

		void hit(Bullet& bullet, RealityEntity& entity, Geom::Vec2 actualPosition) const override{
			float t = bullet.getTrailLifetime();
			Assets::Effects::genTrailFade(Game::core->effectManager.get(), std::move(bullet.trail), trailWidth, trailEnd)->setColor(trailBegin)
				->setLifetime(t);

			Assets::Effects::sparkLarge->suspendOn(Game::core->effectManager.get())->
				set({actualPosition, 0}, effectColor, 30);
		}

		void draw(const Bullet& bullet) const override{
			using namespace Graphic;
			namespace Draw = Graphic::Draw;

			[[maybe_unused]] ext::Guard<Draw::TextureState, &Draw::TextureState::contextTexture> tf
				{Draw::globalState, Draw::globalState.defaultLightTexture};
			bullet.trail.each(trailWidth, Graphic::Trail::DefDraw_WithLerp(trailBegin, trailEnd));

			Draw::color(effectColor);
			Game::Draw::hitbox<Graphic::BatchWorld>(bullet.hitBox);
		}

		void despawn(Bullet& bullet) const override{
			float t = bullet.getTrailLifetime();
			Assets::Effects::genTrailFade(Game::core->effectManager.get(), std::move(bullet.trail), trailWidth, trailEnd)->setColor(trailBegin)
				->setLifetime(t);

			Assets::Effects::squareOut->suspendOn(Game::core->effectManager.get())->
				set({bullet.trans.vec, 0}, effectColor, 30);
		}
	} basicBulletType, basicBulletTypeSlow;
}

namespace _{
	struct Temp{
		[[nodiscard]] Temp(){
			Game::Content::basicBulletTypeSlow.initSpeed = 20;
			Game::Content::basicBulletTypeSlow.maximumLifetime = 200;
			Game::Content::basicBulletTypeSlow.trailUpdateSpacing = 3;

		}
	}_t;
}

