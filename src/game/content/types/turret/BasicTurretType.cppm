export module Game.Content.Type.Turret.BasicTurretType;

export import Game.Entity.Turrets;

import Graphic.Draw;
import Graphic.Color;
import Core;
import Assets.Sound;
import Geom.Shape.RectBox;
import Geom.Transform;
import Game.Entity.Bullet;
import Game.Entity.EntityManager;
import Game.Content.Type.BasicBulletType;

import std;

export namespace Game::Content{
	using namespace Graphic;

	struct BasicTurretType : TurretTrait{
		void update(TurretEntity* turret) const override{

		}

		void draw(const TurretEntity* turret) const override{
			Draw::color(Colors::RED_DUSK);
			Draw::Fill::poly(turret->getX(), turret->getY(), 3, 32, turret->trans.rot);
		}

		void shoot(TurretEntity* turret, RealityEntity* shooter) const override{
			Core::audio->play(Assets::Sounds::laser5);
			Geom::RectBox box{};
			box.setSize(180, 12);
			box.offset = box.sizeVec2;
			box.offset.mul(-0.5f);

			const auto ptr = Game::EntityManage::obtain<Game::Bullet>();
			ptr->trait = &Game::Content::basicBulletType;
			ptr->trans.rot = turret->trans.rot;
			ptr->trans.vec = turret->trans.vec;

			ptr->velo.vec.setPolar(ptr->trans.rot, 320);
			ptr->hitBox.init(box);
			ptr->physicsBody.inertialMass = 100;
			ptr->damage.materialDamage.fullDamage = 100;
			ptr->shooter = shooter;

			ptr->activate();
			Game::EntityManage::add(ptr);
		}
	} baseTurret;
}
