module;

export module Game.Core;

import OS.ApplicationListener;

export import Game.Entity.EntityManager;
export import Game.UI.OverlayManager;
export import Game.UI.HitBoxEditor;
export import Graphic.Effect.Manager;
export import Game.ContentLoader;

import std;

import Core.Renderer;

export namespace Game {
	struct  Core : OS::ApplicationListener {
		std::unique_ptr<ContentLoader> contentLoader{std::make_unique<ContentLoader>()};
		//TODO
		std::unique_ptr<OverlayManager> overlayManager{std::make_unique<OverlayManager>()};
		std::unique_ptr<HitBoxEditor> hitBoxEditor{std::make_unique<HitBoxEditor>()};

		std::unique_ptr<Graphic::EffectManager> effectManager{std::make_unique<Graphic::EffectManager>()};

		[[nodiscard]] Core(){
			pauseRestrictable = true;
		}

		void update(const float delta) override {
			//TODO async
			EntityManage::update(delta);
			effectManager->update(delta);
		}

		void updateGlobal(const float delta) override{
			if(overlayManager->activated)overlayManager->updateGlobal(delta);
			if(hitBoxEditor->activated)hitBoxEditor->updateGlobal(delta);
		}

		virtual void drawAboveUI(::Core::Renderer* renderer) const{
			if(overlayManager->activated){
				overlayManager->drawAboveUI(renderer);
			}

			if(hitBoxEditor->activated){
				hitBoxEditor->drawAboveUI(renderer);
			}
		}

		virtual void drawBeneathUI(::Core::Renderer* renderer) const{
			if(overlayManager->activated){
				overlayManager->drawBeneathUI(renderer);
			}

			if(hitBoxEditor->activated){
				hitBoxEditor->drawBeneathUI(renderer);
			}
		}
	};

	inline std::unique_ptr<Core> core{nullptr};
}
