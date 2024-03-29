//
// Created by Matrix on 2024/3/23.
//

export module Game.Content.Type.SpaceCraft.BasicSpaceCraftType;

export import Game.Entity.SpaceCraft;
export import Game.Content.Drawer.DrawComponents;
export import Game.Content.ContentType;

import std;

export namespace Game::Content{
	struct BasicSpaceCraftType : SpaceCraftTrait, Game::ContentTrait{
		std::unique_ptr<Game::Drawer::DrawComponent<SpaceCraft>> drawer{nullptr}; //I believe drawers should be unique!

		explicit BasicSpaceCraftType(const std::string_view name)
			: ContentTrait(name){}

		BasicSpaceCraftType(const std::string_view name, Concepts::Invokable<void(BasicSpaceCraftType*)> auto&& func) : ContentTrait(name){
			func(this);
		}

		void init(SpaceCraft* spaceCraft) const override{

		}

		void update(SpaceCraft* spaceCraft) const override{

		}

		void draw(const SpaceCraft* entity) const override{
			drawer->draw({Game::Drawer::getCompPos(entity), {.lifetimeProgress = 1.0f}}, entity);
		}

		void pullLoadRequest(Graphic::TextureAtlas& atlas, const OS::FileTree& searchTree, std::string prefix) override{
			prefix.append(name);

			drawer->pullLoadRequest(atlas, searchTree, prefix);
		}
	};
}