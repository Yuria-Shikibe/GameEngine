export module Game.Content.Type.Turret.BasicTurretType;

export import Game.Entity.Turret;

import Graphic.Draw;
import Graphic.Color;
import Core;
import Assets.Sound;
import Geom.Shape.RectBox;
import Geom.Transform;
import Game.Entity.Bullet;
import Game.Entity.EntityManager;
import Game.Content.Type.BasicBulletType;
import Game.Content.Drawer.DrawComponents;
import Game.Content.ContentType;


import std;

export namespace Game::Content{

	struct BasicTurretType : TurretTrait, Game::ContentTrait{
		std::unique_ptr<Game::Drawer::DrawComponent<TurretEntity>> drawer{nullptr}; //I believe drawers should be unique!
		BulletTrait* bulletTrait{&basicBulletTypeSlow};
		float zOffset = -3.0f;

		[[nodiscard]] explicit BasicTurretType(const std::string_view& name)
			: ContentTrait{name}{}

		void update(TurretEntity* turret) const override{

		}

		void draw(const TurretEntity* turret) const override{
			// Graphic::Draw::color(Graphic::Colors::RED_DUSK);
			// Graphic::Draw::Fill::poly(turret->getX(), turret->getY(), 3, 32, turret->trans.rot);

			if(drawer)drawer->draw({Drawer::PartTrans{turret->trans, zOffset}, {.lifetimeProgress = 1.0f}}, turret);
		}

		void shoot(TurretEntity* turret, RealityEntity* shooter) const override{
			Core::audio->play(Assets::Sounds::laser5, turret->trans.vec.x, turret->trans.vec.y, shooter->zLayer);
			Geom::RectBox box{};
			box.setSize(150, 10);
			box.offset = box.sizeVec2;
			box.offset.mul(-0.5f);

			const auto ptr = Game::EntityManage::obtain<Game::Bullet>();
			ptr->trait = bulletTrait;
			ptr->trans.rot = turret->trans.rot;
			ptr->trans.vec = turret->trans.vec;

			ptr->vel.vec.setPolar(ptr->trans.rot, 1);
			ptr->hitBox.init(box);
			ptr->physicsBody.inertialMass = 100;
			ptr->damage.materialDamage.fullDamage = 100;
			ptr->shooter = shooter;

			ptr->activate();
			Game::EntityManage::add(ptr);
		}

		void pullLoadRequest(Graphic::TextureAtlas& atlas, const OS::FileTree& searchTree, std::string prefix) override{
			prefix.append(name);

			if(drawer)drawer->pullLoadRequest(atlas, searchTree, std::move(prefix));
		}
	} baseTurret{""};
}
