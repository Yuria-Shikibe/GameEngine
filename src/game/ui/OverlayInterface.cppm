//
// Created by Matrix on 2024/3/22.
//

export module Game.UI.OverlayInterface;

export import OS.ApplicationListener;

export import Geom.Vector2D;
export import Core;
import std;


export namespace Game{
	struct OverlayInterface : OS::ApplicationListener {
		Geom::Vec2 mousePos{};
		Geom::Vec2 mousePosNormalized{};
		Geom::Vec2 mouseWorldPos{};

		bool activated{false};

		void updateGlobal(float delta) override{
			mousePos = Core::input.getCursorPos();
			mousePosNormalized = Core::renderer->getNormalized(mousePos);
			mouseWorldPos = Core::camera->getScreenToWorld(mousePosNormalized);
		}

		virtual void drawAboveUI(Core::Renderer* renderer) const{

		}

		virtual void drawBeneathUI(Core::Renderer* renderer) const{

		}

		virtual void activate(){
			activated = true;
		}

		virtual void deactivate(){
			activated = false;
		}
	};
}