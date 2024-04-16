module;

export module Game.UI.OverlayManager;

export import Game.UI.OverlayInterface;

import Game.Entity.RealityEntity;
import Game.Entity;
import Geom.Vector2D;
import Geom.Matrix3D;

import UI.Cursor;
import Graphic.Draw;
import Graphic.Trail;
import Font.GlyphArrangement;
import Assets.Graphic;

import Ctrl.Constants;

import ext.Timer;
import ext.Encoding;

import std;

import OS.ApplicationListenerSetter;

export namespace Game {
	using namespace Graphic;
	class OverlayManager : public Game::OverlayInterface, public Game::RemoveCallalble{
	public:
		std::vector<std::shared_ptr<RealityEntity>> selected{};

		// Trail cursorTrail{50};

		bool assigningRoute = false;

		OverlayManager(){
			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_1, Ctrl::Act_Press,
				Ctrl::Mode_Shift,
			[this] {
				if(activated)assignTarget();
			});

			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_1, Ctrl::Act_Press,
				Ctrl::Mode_Shift | Ctrl::Mode_Ctrl,
			[this] {
				if(activated)assignRoute();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_ESCAPE, Ctrl::Act_Press,
			[this] {
				if(activated)releaseSelected();
			});

			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_2, Ctrl::Act_Press,
				Ctrl::Mode_Shift | Ctrl::Mode_Ctrl,
			[this] {
				if(activated)removeRoute();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_ENTER, Ctrl::Act_Press,
			[this] {
				if(activated)assignRouteEnd();
			});

			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_3, Ctrl::Act_Press,
				Ctrl::Mode_Shift,
			[this] {
				if(activated)assignTurretTarget();
			});
		}

		enum IntervalIndex{
			entityCheck,
			count
		};

		ext::Timer<count> timer{};

		[[nodiscard]] UI::CursorDrawabe& getCursor() const{
			if(Core::input->isPressedKey(Ctrl::KEY_LEFT_SHIFT)){
				return UI::getCursor(UI::CursorType::select);
			}

			return UI::getCursor(UI::CursorType::regular);
		}

		// void drawAboveUI(Core::Renderer* renderer) const override{
		// 	const auto& cursor = getCursor();
		//
		// 	if(Core::input->cursorInbound()){
		// 		Draw::color(Colors::WHITE);
		// 		Batch::beginPorj(Geom::MAT3_IDT);
		// 		Batch::blend(GL::Blendings::Inverse);
		// 		cursor.draw(mousePosNormalized.x, mousePosNormalized.y, Core::renderer->getSize(), 2);
		//
		// 		cursorTrail.each(0.0025f, [](
		// 			const Geom::Vec2 v1, const Geom::Vec2 v2, const Geom::Vec2 v3, const Geom::Vec2 v4,
		// 			const float p1, const float p2
		// 		){
		// 			const Color c1 = Color::createLerp(p1, Colors::CLEAR, Colors::WHITE);
		// 			const Color c2 = Color::createLerp(p2, Colors::CLEAR, Colors::WHITE);
		// 			Draw::quad(Draw::defaultTexture, v1, c1, v2, c1, v3, c2, v4, c2);
		// 		});
		//
		// 		Batch::endPorj();
		// 		Batch::blend();
		// 	}
		// }

		void drawBeneathUI(Core::Renderer* renderer) const override{
			static auto coordText = Font::obtainLayoutPtr();
			renderer->frameBegin(&renderer->effectBuffer);
			for(auto& entity : selected){
				if(!entity->controller->moveCommand.shouldDrawUI())continue;
				Draw::Line::setLineStroke(3 + (entity->controller->moveCommand.isAssigningRoute() ? 0 : 3));
				Draw::color(Colors::SLATE);

				Draw::Line::setLerpColor(Colors::SLATE, Colors::AQUA);
				Draw::Line::beginLineVert();
				Draw::Line::push(entity->controller->moveCommand.route);
				Draw::Line::endLineVert([](const Geom::Vec2 p, const Graphic::Color color){
					Draw::color(color);
					Draw::Fill::square(p.x, p.y, 14, 45);
				});

				auto dest = entity->controller->moveCommand.destination;
				Font::defGlyphParser->parseWith(coordText, std::format(
					"${{scl#[0.85]}}${{color#[eeeeeeff]}}[{:.1f}, {:.1f}]",
				dest.x, dest.y));
				coordText->offset.set(dest).add(45, 45);
				coordText->setAlign(Align::Mode::bottom_left);
				coordText->render();

				const auto next = entity->controller->moveCommand.nextDest();
				Draw::Line::square(next.x, next.y, 25, 45);

				Draw::Line::setLineStroke(5);
				Draw::color(Colors::BRICK);
				Draw::Line::line(entity->controller->moveCommand.curTrans.vec, entity->controller->moveCommand.destination);
			}

			Draw::color(Colors::RED_DUSK);
			for(auto& realityEntity : selected){
				for (auto turretTarget : realityEntity->controller->turretTargets){
					Draw::Line::square(turretTarget.x, turretTarget.y, 32, 45);
				}
			}

			renderer->frameEnd(Assets::PostProcessors::bloom.get());
		}

		void assignTarget() const{
			for(auto& entity : selected){
				if(entity->controller->moveCommand.hasRoute())continue;
				entity->controller->moveCommand.assignTarget(mouseWorldPos);
			}
		}

		void assignRoute(){
			assigningRoute = true;
			for(const auto& entity : selected){
				if(!entity->controller->moveCommand.isAssigningRoute()){
					entity->controller->moveCommand.route.clear();
					entity->controller->moveCommand.setRouteAssigningBegin();
					entity->controller->moveCommand.route.push_back(entity->trans.vec);
				}

				entity->controller->moveCommand.route.push_back(mouseWorldPos);
			}
		}

		void assignTurretTarget() const{
			for(const auto& entity : selected){
				entity->controller->assignTurretTarget(std::vector{mouseWorldPos});
			}
		}

		void postRemovePrimitive(Game::Entity* entity) override{
			std::erase_if(selected, [entity](const decltype(selected)::value_type& ptr){
				return ptr.get() == entity;
			});
		}

		void removeRoute(){
			assigningRoute = true;
			for(const auto& entity : selected){
				if(!entity->controller->moveCommand.isAssigningRoute()){
					entity->controller->moveCommand.route.clear();
				}else{
					entity->controller->moveCommand.route.pop_back();
				}
			}
		}

		void assignRouteEnd(){
			if(!assigningRoute)return;
			assigningRoute = false;
			for(const auto& entity : selected){
				entity->controller->moveCommand.setRouteAssigningEnd();
				entity->controller->moveCommand.assignTarget(entity->controller->moveCommand.route.back());
			}
		}

		void update(float delta) override{

		}

		void updateGlobal(float delta) override{
			OverlayInterface::updateGlobal(delta);

			// cursorTrail.update(mousePosNormalized.x, mousePosNormalized.y);


			//TODO optimize this
			timer.run<IntervalIndex::entityCheck>(20, delta, [this]{
				std::erase_if(selected, [](const decltype(selected)::value_type& ptr){
					return ptr->deletable();
				});
			});
		}

		void registerSelected(std::shared_ptr<RealityEntity> entity){
			if(entity->controller->selected){
				selected.push_back(std::move(entity));
			}else{
				std::erase(selected, entity);
			}
		}

		void releaseSelected(){
			for(const auto& entity : selected){
				entity->controller->selected = false;
			}
			selected.clear();
		}

		[[nodiscard]] Geom::Vec2 getMouseInWorld() const{
			return mouseWorldPos;
		}

		void activate() override{
			OverlayInterface::activate();
		}

		void deactivate() override{
			OverlayInterface::deactivate();
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
