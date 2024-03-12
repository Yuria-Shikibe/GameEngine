//
// Created by Matrix on 2024/3/10.
//

export module Game.Content.Type.BasicBulletType;

import Assets.Effects;

import Game.Entity.Bullet;
import Game.Core;
import Graphic.Draw;

export import Game.Delay;

export namespace Game::Content{
	struct BasicBulletType : BulletTrait {
		void update(Bullet& bullet) const override{

		}

		void onShoot(Bullet& bullet) const override{
			Assets::Effects::CircleDrawer.suspendOn(Game::core->effectManager.get())->
				set(bullet.position, 0, Graphic::Colors::SKY);
		}

		void hit(Bullet& bullet, RealityEntity& entity, Geom::Vec2 actualPosition) const override{
			Assets::Effects::squareOut.suspendOn(Game::core->effectManager.get())->
				set(actualPosition, 0, Graphic::Colors::SKY, 30);
		}

		void draw(const Bullet& bullet) const override{
			namespace Draw = Graphic::Draw;
			Draw::color(Graphic::Colors::RED_DUSK);
			Draw::quad(Draw::defaultTexture, bullet.hitBox.v0, bullet.hitBox.v1, bullet.hitBox.v2, bullet.hitBox.v3);
		}

		void despawn(Bullet& bullet) const override{
			Assets::Effects::squareOut.suspendOn(Game::core->effectManager.get())->
				set(bullet.position, 0, Graphic::Colors::SKY, 30);
		}
	} base;
}
