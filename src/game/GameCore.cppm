module;

export module Game.Core;

import OS.ApplicationListener;

export import Game.Entity.EntityManager;
export import Game.UI.OverlayManager;

import <memory>;

export namespace Game {
	class Core : public OS::ApplicationListener {
	public:
		std::unique_ptr<OverlayManager> overlayManager{std::make_unique<OverlayManager>()};

		[[nodiscard]] Core(){
			pauseRestrictable = true;
		}

		void update(const float delta) override {
			EntityManage::update(delta);
		}

		void updateGlobal(const float delta) override{
			overlayManager->updateGlobal(delta);
		}
	};
}
