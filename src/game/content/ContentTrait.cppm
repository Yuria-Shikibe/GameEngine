//
// Created by Matrix on 2024/3/15.
//

export module Game.Content.ContentType;

export import Game.Content.Loadable;
export import ext.Concepts;

import std;

export namespace Game{
	enum class ContentType {
		turret,
		spaceCraft,
		bullet,
	};

	struct ContentTrait : Loadable{
		std::string name{};

		unsigned int id{0};

		~ContentTrait() override = default;

		ContentTrait() = default;

		explicit ContentTrait(const std::string_view name)
			: name(name){
		}
	};

}
