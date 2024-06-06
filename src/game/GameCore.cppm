module;

export module Game.Core;

import OS.ApplicationListener;

export import Game.Entity.EntityManager;
export import Game.UI.OverlayManager;
export import Game.UI.HitBoxEditorLegacy;
export import Graphic.Effect.Manager;
export import Game.ContentLoader;
export import Game.Entity.Controller.Player;

import std;

import Core;
import OS.Ctrl.Bind;

namespace FrameCore = ::Core;

export namespace Game {
	struct Core;

	inline std::unique_ptr<Core> core{nullptr};

	struct Core : OS::ApplicationListener {
		OS::InputBindGroup gameBinds{};

		std::unique_ptr<ContentLoader> contentLoader{std::make_unique<ContentLoader>()};
		//TODO
		std::unique_ptr<OverlayManager> overlayManager{std::make_unique<OverlayManager>()};
		std::unique_ptr<HitBoxEditor> hitBoxEditor{std::make_unique<HitBoxEditor>()};

		std::unique_ptr<Graphic::EffectManager> effectManager{std::make_unique<Graphic::EffectManager>()};

		PlayerController* playerController{nullptr};
		bool cameraLock{false};

		[[nodiscard]] Core(){
			pauseRestrictable = true;
			FrameCore::input.registerSubInput(&gameBinds);
		}

		~Core() override{
			FrameCore::input.eraseSubInput(&gameBinds);
		}

		void sendPlayerMoveAct(const Geom::Vec2 vec2) const{
			if(playerController)playerController->setMoveDirection(vec2);
		}

		void activeBinds() const{
			FrameCore::Util::activeBinds(&gameBinds);
		}

		void deactiveBinds() const{
			FrameCore::Util::deactiveBinds(&gameBinds);
		}

		void reBindPlayerController(PlayerController* controller){
			if(controller && playerController){
				throw ext::IllegalArguments{"Conflicted Player Controller Rebind!"};
			}

			playerController = controller;
		}

		void reBindPlayerController(std::nullptr_t){
			playerController = nullptr;
		}

		void update(::Core::Tick delta) override {
			//TODO better async
			std::future<void> fut{};
			if(!effectManager->activatedEmpty()){
				fut = std::async(&EffectManager::update, effectManager.get(), delta);
			}
			EntityManage::update(delta);
			if(fut.valid())fut.get();
		}

		void updateGlobal(const ::Core::Tick delta) override{
			if(overlayManager->activated)overlayManager->updateGlobal(delta);
			if(hitBoxEditor->activated)hitBoxEditor->updateGlobal(delta);

			if(cameraLock && playerController){
				FrameCore::camera->setPosition(FrameCore::camera->getPosition().lerp(playerController->moveCommand.curTrans.vec, 0.05f));
			}
		}

		virtual void drawAboveUI(FrameCore::Renderer* renderer) const{
			if(overlayManager->activated){
				overlayManager->drawAboveUI(renderer);
			}

			if(hitBoxEditor->activated){
				hitBoxEditor->drawAboveUI(renderer);
			}
		}

		virtual void drawBeneathUI(FrameCore::Renderer* renderer) const{
			if(overlayManager->activated){
				overlayManager->drawBeneathUI(renderer);
			}

			if(hitBoxEditor->activated){
				hitBoxEditor->drawBeneathUI(renderer);
			}
		}
	};
}

export extern Graphic::EffectManager* Graphic::getDefManager(){
	return Game::core->effectManager.get();
}
