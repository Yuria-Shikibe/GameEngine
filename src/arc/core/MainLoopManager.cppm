export module Core.MainLoopManager;

import OS.ApplicationListener;

import Core;

export namespace Core{
	class MainLoopManager{
		std::vector<::OS::ApplicationListener*> applicationListeners{};

		::OS::ApplicationListener* gameCore{};

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
			std::future<void> gameFuture;
			if(!isPaused())gameFuture = std::async(&OS::ApplicationListener::update, gameCore, getDeltaTick());

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

			if(Core::uiRoot)Core::uiRoot->update(getDeltaTick());
			Core::input.update(getDeltaTick());
			if(Core::camera)Core::camera->update(getDeltaTick());

			if(gameFuture.valid())gameFuture.get();
		}

		static float getDeltaTick() noexcept;
		static float getUpdateDeltaTick() noexcept;
		static bool isPaused() noexcept;
		static void handleAsync();
	};

	std::unique_ptr<MainLoopManager> loopManager{std::make_unique<MainLoopManager>()};
}


