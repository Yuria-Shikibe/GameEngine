//
// Created by Matrix on 2024/3/15.
//

export module Game.ContentType;

export import Graphic.TextureAtlas;

import <string_view>;

export namespace Game{
	enum class ContentType {
		turret,
		spaceCraft,
		bullet,
	};

	struct Content {
		std::string_view name{}; //Should be static string, I believe!

		virtual ~Content() = default;

		Content() = default;

		explicit Content(const std::string_view& name)
			: name(name){
		}

		virtual void pullRequest(Graphic::TextureAtlas* atlas) = 0;
	};
}
