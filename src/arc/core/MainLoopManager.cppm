export module Core.MainLoopManager;

import OS.ApplicationListener;

import Core;

export namespace Core{
	class MainLoopManager{
		std::vector<::OS::ApplicationListener*> applicationListeners{};

		OS::ApplicationListener* gameCore{};
		std::binary_semaphore updateBeginSemaphore{0};
		std::binary_semaphore updateEndSemaphore{0};

		std::jthread taskThread{&MainLoopManager::updateTask, this};

		void updateTask(){
			while(!taskThread.get_stop_token().stop_requested()){
				updateBeginSemaphore.acquire();
				if(gameCore)gameCore->update(getDeltaTick());
				updateEndSemaphore.release();
			}
		}

	public:
		~MainLoopManager(){
			gameCore = nullptr;
			updateBeginSemaphore.release();
		}

		[[nodiscard]] OS::ApplicationListener* getGameCore() const{ return gameCore; }

		void setGameCore(OS::ApplicationListener* const gameCore){ this->gameCore = gameCore; }

		/**
		 * \brief Register an application listener.
		 * \param listener Should Has Static Lifetime or at least keeps alive before main loop exited.
		 */
		void registerListener(OS::ApplicationListener* listener){applicationListeners.push_back(listener);}

		void removeListener(OS::ApplicationListener* listener){std::erase(applicationListeners, listener);}

		void clearListeners(){applicationListeners.clear();}

		void update(){
			updateTaskBegin();

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
			if(uiRoot)uiRoot->update(getDeltaTick());
			input.update(getDeltaTick());
			if(camera)camera->update(getDeltaTick());
		}

		void updateTaskBegin(){
			updateBeginSemaphore.release();
		}

		void updateTaskEnd(){
			updateEndSemaphore.acquire();
		}

		static Tick getDeltaTick() noexcept;
		static Tick getUpdateDeltaTick() noexcept;

		static bool isPaused() noexcept;

	private:
		static void handleAsync();
	};

	std::unique_ptr<MainLoopManager> loopManager{std::make_unique<MainLoopManager>()};
}


