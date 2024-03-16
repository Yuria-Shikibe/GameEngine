//
// Created by Matrix on 2024/3/15.
//

export module Game.Drawer.DrawComponents;

export import Game.Drawer.DrawParam;

import GL.Texture.TextureRegionRect;
import Graphic.TextureAtlas;
import Graphic.Draw;

import Concepts;

import <vector>;
import <memory>;

export namespace Game::Drawer{
	struct CompPos {
		Geom::Vec2 offset{};
		float angle{};
		float zOffset{};
	};

	struct DrawComponent  {
		CompPos relaPosition{};

		virtual ~DrawComponent() = default;

		virtual void draw(const DrawParam& param) = 0;

		virtual void load(Graphic::TextureAtlas* atlas){
			// atlas->getPage("").pushRequest()
		}
	};

	struct TextureDrawer : DrawComponent {
		GL::TextureRegionRect* mainRegion{};

		TextureDrawer() = default;

		explicit TextureDrawer(Concepts::Invokable<void(TextureDrawer*)> auto&& func){
			func(this);
		}

		void draw(const DrawParam& param) override{
			Geom::Vec2 pos = this->relaPosition.offset;
			float angle = param.rotation + this->relaPosition.angle;
			float z = param.zLayer + this->relaPosition.zOffset;

			pos.rotate(param.rotation).add(param.position);

			//Graphic::Draw::rect(mainRegion)
		}
	};
}
