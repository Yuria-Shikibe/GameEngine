
export module Assets.Load.Core;

export import Assets.Load.State;
import Core.Async.TaskQueue;
import std;
import ext.algorithm;
import ext.TimeMark;

export namespace Assets::Load{
	using LoadTaskType = void();
	struct LoadTaskHandler;
	class LoadManager;

	struct LoadTaskHandler{
		LoadManager* manager{};

		std::exception_ptr lastException{};
		std::stop_token stopToken{};

		void join();

		void setStopToken(std::stop_token&& token){
			stopToken = std::move(token);
		}

		void setManager(LoadManager* manager);

		[[nodiscard]] Phase getCurrentPhase() const;

		bool tryJoin();

		[[nodiscard]] auto getBarriar() const;

		[[nodiscard]] std::future<void> postTask(std::packaged_task<LoadTaskType>&& task) const;

		[[nodiscard]] std::future<void> postTask(std::invocable<> auto&& task) const{
			return postTask(std::packaged_task<void()>{std::forward<decltype(task)>(task)});
		}

		void throwException(const std::exception_ptr& postException) noexcept;

		explicit operator bool() const noexcept{return !stopToken.stop_requested();}
	};

	class LoadTask{
	protected:
		float progress{};
		std::launch defaultPolicy = std::launch::async;
		LoadTaskHandler handler;

		friend LoadManager;

	public:
		LoadEventManager postEventManager{};

		[[nodiscard]] LoadTask() = default;

		[[nodiscard]] explicit LoadTask(std::launch defaultPolicy)
			: defaultPolicy{defaultPolicy}{}

		void tryArriveAndWait() const noexcept;

		virtual ~LoadTask() = default;

		[[nodiscard]] virtual std::future<void> launch(std::launch policy) = 0;

		[[nodiscard]] std::future<void> launch(){
			return launch(defaultPolicy);
		}

		[[nodiscard]] bool shouldStop() const noexcept{
			return handler.stopToken.stop_requested();
		}

		[[nodiscard]] float getProgress() const noexcept{return progress;}

		[[nodiscard]] virtual std::string_view getCurrentTaskName() const noexcept {return "";}

		//TODO better done setter
		[[nodiscard]] virtual bool isFinished() const noexcept{return handler.stopToken.stop_requested();}
	};


	class LoadManager{
		friend LoadTaskHandler;

		std::vector<LoadTaskHandler*> handlers{};
		std::unordered_map<LoadTask*, std::future<void>> tasks{};

		Core::Async::TaskQueue<LoadTaskType> taskQueue{};
		std::stop_source stopSource{};
		Phase currentPhase{Phase::init};
		static constexpr auto MaxPhaseCount = LoadEventManager::max();

		void afterArrival() noexcept;

		using BarrierType = std::barrier<decltype(std::bind_front(&LoadManager::afterArrival, static_cast<LoadManager*>(nullptr)))>;
		std::unique_ptr<BarrierType> barrier{};

		mutable std::mutex handle_mtx{};
		mutable std::condition_variable cv{};

		ext::StaticTimeStamper<2> timer{};
		static constexpr decltype(timer)::DurTy maxLoadSpacing{330};


		mutable std::ostringstream taskNameBuilder{};
		mutable float taskCurrentProgress{};

	public:
		LoadEventManager eventManager;

		void launch();

		[[nodiscard]] bool isFinished() const noexcept{
			return std::to_underlying(currentPhase) >= MaxPhaseCount;
		}

		float getCurrentProgress() const;

		void registerTask(LoadTask& task);

		void processRequests(){
			tryThrowException();

			taskCurrentProgress = getCurrentProgress();
			if(tasks.empty() || isFinished())return;

			timer.mark();
			while(timer.get() < maxLoadSpacing) {
				auto task = taskQueue.pop();

				if(task){
					task.value()();
				}

			}
		}

		void setupBarrier(){
			barrier = std::make_unique<BarrierType>(tasks.size(), std::bind_front(&LoadManager::afterArrival, this));
		}

		void requestStop(){
			stopSource.request_stop();

			while(!isFinished()){
				(void)barrier->arrive();
			}

			std::unique_lock lk{handle_mtx};
			cv.wait(lk, [this]{return handlers.empty();});
			requestDone();
		}

		void requestDone(){
			//TODO set wait time for timeout handle
			for (auto& [task, future] : tasks){
				if(future.valid())future.get();
			}
		}

		void tryThrowException() const;

		[[nodiscard]] auto& getTaskQueue(){ return taskQueue; }
	};
}

module : private;

namespace Assets::Load{

	auto LoadTaskHandler::getBarriar() const {
		if(stopToken.stop_requested() || !manager){
			return static_cast<std::remove_pointer_t<decltype(manager)>::BarrierType*>(nullptr);
		}
		return manager->barrier.get();
	}

	void LoadTaskHandler::join(){
		if(!manager)return;
		std::unique_lock lk{manager->handle_mtx};
		ext::algo::erase_unique_unstable(manager->handlers, this);

		if(const auto b = getBarriar())b->arrive_and_drop();
		if(manager->handlers.empty())manager->cv.notify_one();
	}

	void LoadTaskHandler::setManager(LoadManager* manager){
		if(!manager)return;
		this->manager = manager;
		stopToken = manager->stopSource.get_token();
	}

	Phase LoadTaskHandler::getCurrentPhase() const{
		if(manager)return manager->currentPhase;
		return Phase::clear;
	}

	bool LoadTaskHandler::tryJoin(){
		if(stopToken.stop_requested()){
			join();
			return true;
		}
		return false;
	}

	std::future<void> LoadTaskHandler::postTask(std::packaged_task<LoadTaskType>&& task) const{

		if(stopToken.stop_requested() || !manager){
			task.operator()();
			return task.get_future();
		}else{
			return manager->getTaskQueue().push(std::move(task));
		}
	}

	void LoadTaskHandler::throwException(const std::exception_ptr& postException) noexcept{
		if(stopToken.stop_requested()){
			std::rethrow_exception(postException);
		}else{
			lastException = postException;
		}
	}

	void LoadTask::tryArriveAndWait() const noexcept{
		if(const auto barriar = handler.getBarriar()){
			barriar->arrive_and_wait();
		}
	}

	void LoadManager::afterArrival() noexcept{
		if(std::to_underlying(currentPhase) < MaxPhaseCount){
			std::println("Phase {} Reached | {:.1f}% Completed.", std::to_underlying(currentPhase), getCurrentProgress() * 100.f);
			std::cout.flush();
			for (const auto task : tasks | std::views::keys){
				task->postEventManager.fire(currentPhase);
			}

			eventManager.fire(currentPhase);

			currentPhase = Phase{1 + std::to_underlying(currentPhase)};
		}else{
			stopSource.request_stop();
		}
	}

	void LoadManager::launch(){
		std::println("Launch Assets Load | Main Thread: {}", std::this_thread::get_id());
		setupBarrier();

		for (auto& [task, future] : tasks){
			task->handler.setManager(this);
			future = task->launch();
		}
	}

	float LoadManager::getCurrentProgress() const{
		float sum{};

		for(const auto task : tasks | std::views::keys) {
			sum += task->getProgress();
		}

		sum /= static_cast<float>(tasks.size());

		return std::clamp(sum, 0.f, 1.f);
	}

	void LoadManager::registerTask(LoadTask& task){
		tasks.try_emplace(&task, std::future<void>{});
	}

	void LoadManager::tryThrowException() const{
		std::scoped_lock lk{handle_mtx};
		for (auto handler : handlers){
			if(handler->lastException){
				std::rethrow_exception(handler->lastException);
			}
		}
	}
}
