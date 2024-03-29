//
// Created by Matrix on 2024/3/15.
//

export module Game.Content.Drawer.TurretDrawer;

export import Game.Entity.Turrets;
export import Game.Content.Drawer.DrawComponents;

import std;

export namespace Game::Drawer{
	struct TurretDrawer {
		virtual ~TurretDrawer() = default;

		std::vector<std::unique_ptr<DrawComponent<TurretEntity>>> components{};

		virtual void paramOperate(DrawParam& param, TurretTrait* trait, TurretEntity* entity){

		}

		virtual void draw(TurretTrait* trait, TurretEntity* entity){
			DrawParam param{};
			paramOperate(param, trait, entity);
			for(auto& component : components){
				component->draw(param, entity);
			}
		}
	};


}