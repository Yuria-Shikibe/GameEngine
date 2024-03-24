//
// Created by Matrix on 2024/3/15.
//
//TODO 模块分区
// Merge these in module *Drawer*
export module Game.Content.Drawer.DrawComponents;

export import Game.Content.Drawer.DrawParam;
export import Game.Content.Loadable;

import Geom.Transform;
import GL.Texture.TextureRegionRect;
import Graphic.TextureAtlas;
import Graphic.Draw;

import Concepts;

import std;

export namespace Game::Drawer{
	struct Movement{
		Geom::Transform trans{};
		std::function<float(const DrawParam&)> interper{nullptr};

		[[nodiscard]] Geom::Transform transBy(const DrawParam& param) const noexcept{
			return trans * interper(param);
		}

		[[nodiscard]] explicit operator bool() const noexcept{
			return interper != nullptr;
		}
	};

	struct DrawComponent : Game::Loadable{
		std::string name{};

		CompPos trans{};
		/** @brief Uses this if possible */
		Movement generalMovement{};
		/** @brief Avoid using this if possible */
		std::vector<Movement> movements{};

		DrawComponent() = default;

		explicit DrawComponent(const std::string_view partName)
			: name(partName){}

		explicit DrawComponent(std::string&& partName)
			: name(std::move(partName)){}

		~DrawComponent() override = default;

		virtual void draw(const DrawParam& param, const BaseEntity* entity) const = 0;
	};

	struct TextureDrawer : DrawComponent {
		GL::TextureRegionRect* mainRegion{};

		Graphic::Color lightColor{Graphic::Colors::WHITE};

		TextureDrawer() = default;

		explicit TextureDrawer(const std::string_view partName, Concepts::Invokable<void(TextureDrawer*)> auto&& func) :
			DrawComponent(partName)
		{
			func(this);
		}

		void pullLoadRequest(Graphic::TextureAtlas& atlas, const OS::FileTree& searchTree, std::string prefix) override{
			prefix.append(name);

			mainRegion = atlas.getPage("base").pushRequest(prefix, searchTree.flatFind<true>(prefix));
			// atlas->getPage("base").pushRequest(prefix, searchTree.flatFind(prefix));
			atlas.getPage("light").pushRequest(prefix, searchTree.flatFind<true>(prefix + ".light"));
		}

		void draw(const DrawParam& param, const BaseEntity* entity) const override{
			using namespace Graphic;
			auto cur = trans | param.trans;

			if(generalMovement){
				cur.Geom::Transform::operator+=(generalMovement.transBy(param));
			}

			[[unlikely]] if(!movements.empty()){
				for(auto& move : movements){
					cur.Geom::Transform::operator+=(move.transBy(param));
				}
			}

			Draw::color(lightColor);
			Draw::setZ(cur.zOffset);
			Draw::rect<WorldBatch>(mainRegion, cur);
		}
	};
}
