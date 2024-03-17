module;

export module Game.Core;

import OS.ApplicationListener;

export import Game.Entity.EntityManager;
export import Game.UI.OverlayManager;
export import Graphic.Effect.Manager;

import std;

export namespace Game {
	class Core : public OS::ApplicationListener {
	public:
		//TODO why pointer???
		std::unique_ptr<OverlayManager> overlayManager{std::make_unique<OverlayManager>()};
		std::unique_ptr<Graphic::EffectManager> effectManager{std::make_unique<Graphic::EffectManager>()};

		[[nodiscard]] Core(){
			pauseRestrictable = true;
		}

		void update(const float delta) override {
			EntityManage::update(delta);
			effectManager->update(delta);
		}

		void updateGlobal(const float delta) override{
			overlayManager->updateGlobal(delta);
		}
	};

	inline std::unique_ptr<Core> core{nullptr};
}
