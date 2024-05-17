//
// Created by Matrix on 2024/5/16.
//

export module SideTemp;

import Assets.Directories;

import Game.Core;
import Game.Content.Builtin.Turrets;
import Game.Content.Builtin.SpaceCrafts;
import Game.Pool;
import Game.Entity.Collision;
import Game.Entity.Controller;
import Game.Entity.Controller.Player;
import Game.Entity.Controller.AI;

import Math.Rand;

export namespace Test{
	void genRandomEntities(){
		Game::score = 0;
		Game::EntityManage::clear();
		Math::Rand rand = Math::globalRand;
		for(int i = 0; i < 300; ++i) {


			const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
			ptr->trans.rot  = 0.0f;//rand.random(0.0f);
			Game::read(OS::File{Assets::Dir::game.subFile(R"(hitbox\macrophage.hitbox)")}, ptr->hitBox);
			ptr->trans.vec.set(rand.range(20000), rand.range(20000));
			ptr->setHealth(5000);
			ptr->initTrait(Game::Content::Builtin::test_macrophage);

			Game::EntityManage::add(ptr);
			ptr->setHealth(500);
			ptr->physicsBody.inertialMass = 500;
			ptr->chambers.setLocalTrans(ptr->trans);
			ptr->activate();


			ptr->init();
			ptr->setTurretType(Game::Content::Builtin::test_pesterSecondary);
			ptr->controller.reset(new Game::AiController{ptr.get()});

		}

		const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
		ptr->trans.vec.set(0, 0);
		Game::EntityManage::add(ptr);
		Game::read(OS::File{Assets::Dir::game.subFile(R"(hitbox\pester.hitbox)")}, ptr->hitBox);

		std::array posArr{Geom::Vec2{-330, 100}, Geom::Vec2{-100, 200}, Geom::Vec2{400, 130}};

		for (auto arr : posArr){
			for(const int i : {-1, 1}){
				ptr->turretEntities.push_back(Game::EntityManage::obtainUnique<Game::TurretEntity>());
				auto& t = ptr->turretEntities.back().operator*();

				t.relativePosition += arr * Geom::Vec2(1, i);
				t.init(Game::Content::Builtin::test_pulseLaser, ptr.get());
			}
		}

		ptr->vel.vec.set(0, 0);
		ptr->setHealth(20000);
		ptr->setHealthMaximum(20000);
		ptr->initTrait(Game::Content::Builtin::test_pester);
		ptr->activate();
		// ptr->chambers.operator=(std::move(*chamberFrame));
		// ptr->chamberTrans.vec.x = 85;
		ptr->physicsBody.inertialMass = 4000;
		// chamberFrame = std::make_unique<Game::ChamberFrameTrans<Game::SpaceCraft>>();
		ptr->controller.reset(new Game::PlayerController{ptr.get()});

	}
}
