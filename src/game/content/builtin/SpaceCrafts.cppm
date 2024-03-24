//
// Created by Matrix on 2024/3/24.
//

export module Game.Content.Builtin.SpaceCrafts;

export import Game.Content.Type.SpaceCraft.BasicSpaceCraftType;
import Game.Content.Drawer.DrawComponents;
import Game.ContentLoader;

import std;

export namespace Game::Content::Builtin{
	Game::SpaceCraftTrait*
		test{};

	void load_SpaceCraft(Game::ContentLoader* loader){
		test = loader->registerContent<BasicSpaceCraftType>("pester", [](BasicSpaceCraftType* self){
			self->drawer = std::make_unique<Game::Drawer::TextureDrawer>("", [](Game::Drawer::TextureDrawer* drawer){
				drawer->trans.pos.x = 100;
			});
		});

	}
}
