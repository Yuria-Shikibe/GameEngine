module;

export module Game.Core;

import OS.ApplicationListener;

import Game.Entity.EntityManager;

export namespace Game {
	class Core : public OS::ApplicationListener {
	public:
		void update(const float delta) override {
			EntityManage::update(delta);
		}
	};
}
