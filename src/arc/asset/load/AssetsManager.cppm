//
// Created by Matrix on 2024/6/10.
//

export module Assets.Load.Manager;

import Graphic.TextureAtlas;
import Font.Manager;

export namespace Assets::Load{
	struct Manager{
		Font::FontManager fonts{};
		Graphic::TextureAtlas atlas{};
	};
}
