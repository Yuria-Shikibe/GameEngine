export module Game.Content.Drawer.DrawComponents;

//
// Created by Matrix on 2024/3/15.
//
//TODO 模块分区
// Merge these in module *Drawer*

export import Game.Content.Drawer.DrawParam;
export import Game.Content.Loadable;

import Geom.Transform;
import GL.Texture.TextureRegionRect;
import Graphic.TextureAtlas;
import Graphic.Draw;

import ext.Concepts;

import std;

export namespace Game::Drawer{
	struct PartMovement{
		Geom::Transform trans{};
		std::function<float(const DrawParam&)> interper{nullptr};

		[[nodiscard]] Geom::Transform apply(const DrawParam& param) const noexcept{
			return trans * interper(param);
		}

		[[nodiscard]] explicit operator bool() const noexcept{
			return interper != nullptr;
		}
	};

	template <Concepts::Derived<BaseEntity> T>
	struct DrawComponent : Game::Loadable{
		std::string name{};

		PartTrans trans{};
		/** @brief Uses this if possible */
		PartMovement generalMovement{};
		/** @brief Avoid using this if possible */
		std::vector<PartMovement> movements{};

		DrawComponent() = default;

		explicit DrawComponent(const std::string_view partName)
			: name(partName){}

		explicit DrawComponent(std::string&& partName)
			: name(std::move(partName)){}

		~DrawComponent() override = default;

		virtual void draw(const DrawParam& param, const T* entity) const = 0;

		void passTrans(const DrawParam& param, PartTrans& trans) const{
			if(generalMovement){
				trans.Geom::Transform::operator+=(generalMovement.apply(param));
			}

			if(!movements.empty()) [[unlikely]]  {
				for(auto& move : movements){
					trans.Geom::Transform::operator+=(move.apply(param));
				}
			}
		}
	};

	template <Concepts::Derived<BaseEntity> T>
	struct TextureDrawer : DrawComponent<T>{
		GL::TextureRegionRect* mainRegion{};

		Geom::Vec2 scl = Geom::norBaseVec2<float>;
		Graphic::Color lightColor{Graphic::Colors::WHITE};

		using DrawComponent<T>::generalMovement;
		using DrawComponent<T>::movements;
		using DrawComponent<T>::name;
		using DrawComponent<T>::trans;

		TextureDrawer() = default;

		explicit TextureDrawer(const std::string_view partName, Concepts::Invokable<void(TextureDrawer*)> auto&& func) :
			DrawComponent<T>(partName)
		{
			func(this);
		}

		void pullLoadRequest(Graphic::TextureAtlas& atlas, const OS::FileTree& searchTree, std::string prefix) override{
			prefix.append(name);

			mainRegion = atlas.getPage("base").pushRequest(prefix, searchTree.flatFind<true>(prefix));
			// atlas->getPage("base").pushRequest(prefix, searchTree.flatFind(prefix));
			atlas.getPage("light").pushRequest(prefix, searchTree.flatFind<true>(prefix + ".light"));
		}

		void draw(const DrawParam& param, const T* entity) const override{
			using namespace Graphic;
			PartTrans cur = trans | param.trans;

			this->passTrans(param, cur);

			Draw::color(lightColor);
			Draw::setZ(cur.zOffset);
			Draw::rect<BatchWorld>(mainRegion, cur, scl);
		}
	};

	template <Concepts::Derived<BaseEntity> T>
	struct MultiDrawer : DrawComponent<T>{
		using DrawComponent<T>::generalMovement;
		using DrawComponent<T>::movements;
		using DrawComponent<T>::name;
		using DrawComponent<T>::trans;

		std::vector<std::unique_ptr<DrawComponent<T>>> drawers{};

		MultiDrawer() = default;

		explicit MultiDrawer(const std::string_view groupName, std::initializer_list<std::unique_ptr<DrawComponent<T>>> drawers, Concepts::InvokeNullable<void(MultiDrawer*)> auto&& func = nullptr) :
			DrawComponent<T>(groupName), drawers(std::move(drawers))
		{
			if constexpr (!std::same_as<decltype(func), std::nullptr_t>){
				func(this);
			}
		}

		void draw(const DrawParam& param, const T* entity) const override{
			auto curParam = param;
			curParam.trans |= param.trans;

			this->passTrans(param, curParam.trans);

			for (const auto& drawer : drawers){
				drawer->draw(curParam, entity);
			}
		}

		void pullLoadRequest(Graphic::TextureAtlas& atlas, const OS::FileTree& searchTree, std::string prefix) override{
			prefix.append(name).append(".");

			for (const auto& drawer : drawers){
				drawer->pullLoadRequest(atlas, searchTree, prefix);
			}
		}
	};
}
