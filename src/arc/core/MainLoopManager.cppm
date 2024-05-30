export module Core.MainLoopManager;

import OS.ApplicationListener;

import Core;

export namespace Core{
	class MainLoopManager{
		std::vector<::OS::ApplicationListener*> applicationListeners{};

		::OS::ApplicationListener* gameCore{};
		mutable std::future<void> gameLoopFuture{};

	public:
		[[nodiscard]] OS::ApplicationListener* getGameCore() const{ return gameCore; }

		void setGameCore(OS::ApplicationListener* const gameCore){ this->gameCore = gameCore; }

		/**
		 * \brief Register an application listener.
		 * \param listener Should Has Static Lifetime or at least keeps alive before main loop exited.
		 */
		void registerListener(OS::ApplicationListener* listener){applicationListeners.push_back(listener);}

		void removeListener(OS::ApplicationListener* listener){std::erase(applicationListeners, listener);}

		void clearListeners(){applicationListeners.clear();}

		void update() const{
			if(gameLoopFuture.valid())gameLoopFuture.get();
			if(!isPaused() && gameCore)gameLoopFuture = std::async(&OS::ApplicationListener::update, gameCore, getDeltaTick());

			handleAsync();

			for(const auto listener : applicationListeners){
				listener->updateGlobal(getDeltaTick());

				if(listener->pauseRestrictable) {
					if(!isPaused())listener->update(getUpdateDeltaTick());
				}else {
					listener->update(getDeltaTick());
				}
			}

			for(const auto listener : applicationListeners){
				listener->updatePost(getDeltaTick());
			}

			if(gameCore)gameCore->updateGlobal(getDeltaTick());
			if(Core::uiRoot)Core::uiRoot->update(getDeltaTick());
			Core::input.update(getDeltaTick());
			if(Core::camera)Core::camera->update(getDeltaTick());
		}

		static Core::Tick getDeltaTick() noexcept;
		static Core::Tick getUpdateDeltaTick() noexcept;
		static bool isPaused() noexcept;
		static void handleAsync();
	};

	std::unique_ptr<MainLoopManager> loopManager{std::make_unique<MainLoopManager>()};
}


