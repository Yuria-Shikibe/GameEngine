//
// Created by Matrix on 2024/3/10.
//

export module Game.Content.Type.BasicBulletType;

import Assets.Effects;

import Game.Entity.Bullet;
import Game.Core;
import Graphic.Draw;

import Game.Graphic.Draw;

export import Game.Delay;

export namespace Game::Content{
	struct BasicBulletType : BulletTrait {
		void update(Bullet& bullet) const override{

		}

		void onShoot(Bullet& bullet) const override{
			Assets::Effects::circleOut->suspendOn(Game::core->effectManager.get())->
				set({.vec = bullet.trans.vec}, Graphic::Colors::RED_DUSK);
		}

		void hit(Bullet& bullet, RealityEntity& entity, Geom::Vec2 actualPosition) const override{
			Assets::Effects::sparkLarge->suspendOn(Game::core->effectManager.get())->
				set({actualPosition, 0}, Graphic::Colors::RED_DUSK, 30);
		}

		void draw(const Bullet& bullet) const override{
			namespace Draw = Graphic::Draw;

			Draw::color(Graphic::Colors::RED_DUSK);
			Graphic::Draw::Game::hitbox(bullet.hitBox);
			// Draw::quad(Draw::defaultTexture, bullet.hitBox.v0, bullet.hitBox.v1, bullet.hitBox.v2, bullet.hitBox.v3);
		}

		void despawn(Bullet& bullet) const override{
			Assets::Effects::squareOut->suspendOn(Game::core->effectManager.get())->
				set({bullet.trans.vec, 0}, Graphic::Colors::RED_DUSK, 30);
		}
	} basicBulletType;
}
