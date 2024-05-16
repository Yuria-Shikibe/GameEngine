//
// Created by Matrix on 2024/3/24.
//

export module Game.Content.Builtin.SpaceCrafts;

export import Game.Content.Type.SpaceCraft.BasicSpaceCraftType;
import Game.Content.Drawer.DrawComponents;
import Game.ContentLoader;

import std;

export namespace Game::Content::Builtin{
	Game::SpaceCraftTrait
		*test_pester{},
		*test_macrophage{}
	;

	void load_SpaceCraft(Game::ContentLoader* loader){
		test_pester = loader->registerContent<BasicSpaceCraftType>("pester", [](BasicSpaceCraftType& self){
			self.maximumSpeed = 30.0f;
			self.drawer = std::make_unique<Game::Drawer::TextureDrawer<Game::SpaceCraft>>("",
			[](Game::Drawer::TextureDrawer<Game::SpaceCraft>* drawer){
				drawer->trans.vec.x = 100;
			});
		});

		test_macrophage = loader->registerContent<BasicSpaceCraftType>("macrophage", [](BasicSpaceCraftType& self){
			self.drawer = std::make_unique<Game::Drawer::TextureDrawer<Game::SpaceCraft>>("",
			[](Game::Drawer::TextureDrawer<Game::SpaceCraft>* drawer){
				drawer->trans.vec.x = 100;
			});
		});
	}
}
