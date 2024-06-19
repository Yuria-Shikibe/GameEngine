export module Core.MainLoopManager;

import OS.ApplicationListener;

import Core;
import Core.ApplicationTimer;
import std;

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
				if(gameCore)gameCore->update(timer.isPaused() ? Core::Tick{0.f} : timer.getUpdateDeltaTick());
				updateEndSemaphore.release();
			}
		}

		std::thread::id mainThreadID{};

		Core::Sec FPS_reload = 0;
		unsigned totalFrames = 0;
		unsigned FPS_last = 0;

		void updateFPS(){
			FPS_reload += timer.getDeltaTick();

			++totalFrames;
			if(FPS_reload > 1) {
				FPS_last = totalFrames;
				FPS_reload = 0.0f;
				totalFrames = 0u;
			}
		}

	public:
		ApplicationTimer timer{};

		[[nodiscard]] MainLoopManager() : mainThreadID{std::this_thread::get_id()}{}

		~MainLoopManager(){
			gameCore = nullptr;
			updateBeginSemaphore.release();
		}

		[[nodiscard]] constexpr unsigned int getFPS() const noexcept{ return FPS_last; }

		[[nodiscard]] OS::ApplicationListener* getGameCore() const{ return gameCore; }

		void setGameCore(OS::ApplicationListener* const gameCore){ this->gameCore = gameCore; }

		/**
		 * \brief Register an application listener.
		 * \param listener Should Has Static Lifetime or at least keeps alive before main loop exited.
		 */
		void registerListener(OS::ApplicationListener* listener){applicationListeners.push_back(listener);}

		void removeListener(OS::ApplicationListener* listener){std::erase(applicationListeners, listener);}

		void clearListeners(){applicationListeners.clear();}

		void updateMisc(){
			for(const auto listener : applicationListeners){
				listener->updateGlobal(timer.delta());

				if(listener->pauseRestrictable) {
					if(!timer.isPaused())listener->update(timer.getUpdateTick());
				}else {
					listener->update(timer.delta());
				}
			}

			for(const auto listener : applicationListeners){
				listener->updatePost(timer.delta());
			}

			updateFPS();
			if(audio)audio->setListenerPosition(Core::camera->getPosition().x, Core::camera->getPosition().y);

			if(gameCore)gameCore->updateGlobal(timer.delta());
			if(uiRoot)uiRoot->update(timer.delta());
			input.update(timer.delta());
			if(camera){
				camera->update(timer.delta());
				audio->setListenerPosition(camera->getPosition().x, camera->getPosition().y);
			}
		}

		void updateTaskBegin(){
			updateBeginSemaphore.release();
		}

		void updateTaskEnd(){
			updateEndSemaphore.acquire();
			gameCore->updatePost(0);
		}

		constexpr Tick delta() const noexcept{
			return timer.delta();
		}

	};

	std::unique_ptr<MainLoopManager> loopManager{};
}


