module;

export module Game.Core;

import OS.ApplicationListener;

export import Game.Entity.EntityManager;
export import Game.UI.OverlayRenderer;

import <memory>;

export namespace Game {
	class Core : public OS::ApplicationListener {
	public:
		std::unique_ptr<OverlayRenderer> overlayRenderer{std::make_unique<OverlayRenderer>()};

		[[nodiscard]] Core(){
			pauseRestrictable = true;
		}

		void update(const float delta) override {
			EntityManage::update(delta);
		}

		void updateGlobal(const float delta) override{
			overlayRenderer->updateGlobal(delta);
		}
	};
}
