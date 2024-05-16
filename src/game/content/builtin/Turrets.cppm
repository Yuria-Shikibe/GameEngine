export module Game.Content.Builtin.Turrets;

export import Game.Content.Type.Turret.BasicTurretType;
export import Game.Entity.Turret;
import Game.Content.Drawer.DrawComponents;
import Game.ContentLoader;
import Game.Content.Type.BasicBulletType;

import std;

export namespace Game::Content::Builtin{
	Game::TurretTrait
		*test_pulseLaser{},
		*test_pesterSecondary{}
	;

	void load_Turrets(Game::ContentLoader* loader){
		test_pulseLaser = loader->registerContent<BasicTurretType>("ancient-pulse-laser", [](BasicTurretType& self){
			self.reloadTime = 20.0f;
			self.bulletTrait = &Game::Content::basicBulletType;
			self.drawer = std::make_unique<Game::Drawer::TextureDrawer<Game::TurretEntity>>("",
			[](Game::Drawer::TextureDrawer<Game::TurretEntity>* drawer){
				// drawer->trans.vec.x = 100;
			});
		});

		test_pesterSecondary = loader->registerContent<BasicTurretType>("pester-secondary-laser", [](BasicTurretType& self){
			self.drawer = std::make_unique<Game::Drawer::TextureDrawer<Game::TurretEntity>>("",
			[](Game::Drawer::TextureDrawer<Game::TurretEntity>* drawer){
				// drawer->trans.vec.x = 100;
			});
		});
	}
}