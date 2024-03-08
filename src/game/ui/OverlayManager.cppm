module;

export module Game.UI.OverlayManager;

import Game.Entity.RealityEntity;
import Core;
import Geom.Vector2D;
import Geom.Matrix3D;
import OS.ApplicationListener;

import Assets.Cursor;
import Graphic.Draw;
import Graphic.Draw.Lines;
import Graphic.Trail;
import Assets.Graphic;

import Ctrl.Constants;

import Interval;

import <memory>;
import <unordered_map>;
import <ranges>;

export namespace Game {
	using namespace Graphic;
	class OverlayManager : public OS::ApplicationListener{
	public:
		Geom::Vec2 mousePos{};
		Geom::Vec2 mousePosNormalized{};
		Geom::Vec2 mouseWorldPos{};

		std::vector<std::shared_ptr<RealityEntity>> selected{};

		Trail cursorTrail{50};

		bool assigningRoute = false;

		OverlayManager(){
			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_1, Ctrl::Act_Press,
				Ctrl::Mode_Shift,
			[this] {
				assignTarget();
			});

			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_1, Ctrl::Act_Press,
				Ctrl::Mode_Shift | Ctrl::Mode_Ctrl,
			[this] {
				assignRoute();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_ESCAPE, Ctrl::Act_Press,
			[this] {
				releaseSelected();
			});

			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_2, Ctrl::Act_Press,
				Ctrl::Mode_Shift | Ctrl::Mode_Ctrl,
			[this] {
				removeRoute();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_ENTER, Ctrl::Act_Press,
			[this] {
				assignRouteEnd();
			});
		}

		enum IntervalIndex{
			entityCheck,
			count
		};

		ext::Interval<count> timer{};

		void drawAboveUI(Core::Renderer* renderer) const{
			const auto& cursor = getCursor();

			if(Core::input->cursorInbound()){
				Draw::color(Colors::WHITE);
				Core::overlayBatch->beginTempProjection(Geom::MAT3_IDT);
				Draw::blend(GL::Blendings::Inverse);
				cursor.draw(mousePosNormalized.x, mousePosNormalized.y, Core::renderer->getSize(), 2);

				cursorTrail.each(0.0025f, [](
					const Geom::Vec2 v1, const Geom::Vec2 v2, const Geom::Vec2 v3, const Geom::Vec2 v4,
					const float p1, const float p2
				){
					const Color c1 = Color::createLerp(p1, Colors::CLEAR, Colors::WHITE);
					const Color c2 = Color::createLerp(p2, Colors::CLEAR, Colors::WHITE);
					Draw::quad(Draw::defaultTexture, v1, c1, v2, c1, v3, c2, v4, c2);
				});

				Core::overlayBatch->endTempProjection();
				Draw::blend();
			}
		}

		Assets::Cursor& getCursor() const{
			if(Core::input->isPressedKey(Ctrl::KEY_LEFT_SHIFT)){
				return Assets::getCursor(Assets::CursorType::select);
			}

			return Assets::getCursor(Assets::CursorType::regular);
		}

		void drawBeneathUI(Core::Renderer* renderer) const{
			renderer->frameBegin(&renderer->effectBuffer);
			for(auto& entity : selected){
				Draw::setLineStroke(3 + (entity->controller->moveCommand.isAssigningRoute() ? 0 : 3));
				Draw::color(Colors::SLATE);

				Draw::Line::setLerpColor(Colors::RED, Colors::SLATE);
				Draw::Line::beginLineVert();
				Draw::Line::push(
					entity->controller->moveCommand.route | std::ranges::views::take(entity->controller->moveCommand.currentRouteIndex)
				);
				Draw::Line::endLineVert();

				Draw::line(entity->controller->moveCommand.lastDest(), entity->controller->moveCommand.nextDest(), Colors::SLATE, Colors::GREEN);

				Draw::Line::setLerpColor(Colors::SLATE, Colors::AQUA);
				Draw::Line::beginLineVert();
				Draw::Line::push(
					entity->controller->moveCommand.route | std::ranges::views::drop(entity->controller->moveCommand.currentRouteIndex)
				);
				Draw::Line::endLineVert([](const Geom::Vec2 p, const Graphic::Color color){
					Draw::color(color);
					Draw::fillSquare(p.x, p.y, 14, 45);
				});

				const auto next = entity->controller->moveCommand.nextDest();
				Draw::lineSquare(next.x, next.y, 25, 45);

				Draw::setLineStroke(6);
				Draw::color(Colors::BRICK);
				Draw::line(entity->controller->moveCommand.currentPosition, entity->controller->moveCommand.destination);
			}

			renderer->frameEnd(Assets::PostProcessors::bloom);
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
					entity->controller->moveCommand.route.push_back(entity->position);
				}

				entity->controller->moveCommand.route.push_back(mouseWorldPos);
			}
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
			mousePos = Core::input->getMousePos();
			mousePosNormalized = Core::renderer->getNormalized(mousePos);
			mouseWorldPos = Core::camera->getScreenToWorld(mousePosNormalized);

			cursorTrail.update(mousePosNormalized.x, mousePosNormalized.y);


			//TODO optimize this
			timer.run<20, IntervalIndex::entityCheck>(delta, [this]{
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
