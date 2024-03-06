module;

export module Game.UI.OverlayRenderer;

import Game.Entity.RealityEntity;
import Core;
import Geom.Vector2D;
import Geom.Matrix3D;
import OS.ApplicationListener;

import Assets.Cursor;
import Graphic.Draw;

import <memory>;
import <unordered_map>;

export namespace Game {
	using namespace Graphic;
	class OverlayRenderer : public OS::ApplicationListener{
	public:
		Geom::Vec2 mousePos{};
		Geom::Vec2 mousePosNormalized{};
		Geom::Vec2 mouseWorldPos{};

		void drawAboveUI() const{
			const auto& cursor = Assets::getCursor(Assets::CursorType::general);

			if(Core::input->cursorInbound()){
				Draw::color(Colors::WHITE);
				Core::overlayBatch->beginTempProjection(Geom::MAT3_IDT);
				Draw::blend(GL::Blendings::Inverse);
				cursor.draw(mousePosNormalized.x, mousePosNormalized.y, Core::renderer->getSize(), 2);
				Core::overlayBatch->endTempProjection();
				Draw::blend();
			}
		}

		void drawBeneathUI() const{

		}

		void updateGlobal(float delta) override{
			mousePos = Core::input->getMousePos();
			mousePosNormalized = Core::renderer->getNormalized(mousePos);
			mouseWorldPos = Core::camera->getScreenToWorld(mousePosNormalized);
		}

		[[nodiscard]] Geom::Vec2 getMouseInWorld() const{
			return mouseWorldPos;
		}
	};

	//
	// std::unordered_map<IDType, std::weak_ptr<const Game::RealityEntity>> selected{};
	//
	// void addSelect(const Game::RealityEntity* entity) {
	// 	auto ptr = std::dynamic_pointer_cast<const Game::RealityEntity>(entity->obtainSharedSelf());
	// 	selected.emplace(entity->getID(), ptr);
	// }
	//
	//
	// void initCtrl() {
	//
	// }


}
