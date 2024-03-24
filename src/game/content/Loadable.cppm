//
// Created by Matrix on 2024/3/23.
//

export module Game.Content.Loadable;

export import Graphic.TextureAtlas;
export import OS.FileTree;

import std;

export namespace Game{
	struct Loadable{
		virtual ~Loadable() = default;

		/**
		 * @brief
		 * @param searchTree Where To Search The Texture Files
		 * @param atlas Texture Region Atlas
		 * @param prefix Name Suffix For Texture Load
		 */
		virtual void pullLoadRequest(Graphic::TextureAtlas& atlas, const OS::FileTree& searchTree, std::string prefix){

		}
	};
}
