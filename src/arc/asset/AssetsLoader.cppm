export module Assets.Loader;

import Concepts;
import ext.Async;
import ext.TimeMark;
import ext.RuntimeException;
import std;
import OS.GlobalTaskQueue;

export namespace Assets{
	class AssetsLoader;

	enum class LoadType{
		text,
		texture,
		model,
		audio,
	};


	struct AssetsTaskHandler: ext::TaskHandler {
		AssetsLoader* target{nullptr};
		mutable std::mutex lock{};

		[[nodiscard]] explicit AssetsTaskHandler(AssetsLoader* const target)
			: target(target) {
		}

		[[nodiscard]] AssetsTaskHandler() = default;

		std::future<void> operator()(Concepts::Invokable<void()> auto&& func) const;
	};

	class AssetsLoader{
		using TaskFuture = std::future<void>;
		using Handler = AssetsTaskHandler;
		using Task = ext::ProgressTask<void, Handler>*;

		static constexpr auto maxLoadSpacing = ::std::chrono::milliseconds(330);

		ext::Timestamper timer{};
		std::unordered_map<Task, TaskFuture> tasks{};

		std::unique_ptr<Handler> postHandler{std::make_unique<Handler>(this)};

		mutable std::stringstream sstream{};

		float taskProgress{0};
		bool done{false};

	public:
		[[nodiscard]] AssetsLoader() {
			timer.mark();
		}

		~AssetsLoader()  = default;

		std::queue<std::packaged_task<void()>> postedTasks{};

		void begin() {
			// std::cout << "Assets Load Begin" << std::endl;

			OS::activateHander();

			for(auto& [task, future] : tasks) {
				future = task->launch();
			}
		}

		[[nodiscard]] ext::Timestamper& getTimer() {
			return timer;
		}

		void interrupt() {
			forceGet();
		}

		void reset() {
			taskProgress = 0;
			done = false;
		}

		void setDone() {
			done = true;
			tasks.clear();
			OS::deactivateHander();
			std::println(std::cout, "Assets Loader Loop Terminated");
			std::cout.flush();
		}

		[[nodiscard]] bool finished() const {
			return done;
		}

		[[nodiscard]] float getProgress() const {
			return taskProgress;
		}

		/**
		 * \brief Run This In Main Loop Until ProgressTask.finished()
		 */
		void processRequests(){
			timer.mark(1);

			size_t doneCount = 0;
			taskProgress = 0.0f;

			for(auto& task : tasks | std::views::keys) {
				taskProgress += task->getProgress() / static_cast<float>(this->tasks.size());
				if(task->finished())++doneCount;
			}

			if(doneCount == tasks.size()) {
				setDone();
				return;
			}

			if(postedTasks.empty() || done)return;

			auto duration = timer.toMark(1);

			while(duration < maxLoadSpacing) {
				this->postHandler->lock.lock();

				if(postedTasks.empty()) {
					this->postHandler->lock.unlock();
					break;
				}

				auto task = std::move(postedTasks.front());
				postedTasks.pop();

				this->postHandler->lock.unlock();

				try {
					task();
				}catch(...){
					//TODO exception handle
					throw std::current_exception();
				}

				duration = timer.toMark(1);
			}
		}


		void forceGet() {
			while(!finished()) {
				processRequests();
			}
		}

		void push(Task task, const bool asTask = true) {
			task->setHandler(this->postHandler.get());
			if(asTask)tasks.try_emplace(task, TaskFuture{});
		}

		[[nodiscard]] std::string_view getTaskNames(const std::string_view prefix = "", const std::string_view linePrefix = "") const {
			sstream.str("");

			sstream << prefix;

			for(auto& task : tasks | std::ranges::views::keys) {
				if(task->finished())continue;
				sstream << linePrefix << task->getTaskName() << '\n';
			}

			if(!postedTasks.empty()) {
				sstream << linePrefix << "Posted Tasks: " << postedTasks.size() << '\n';
			}

			return sstream.view();
		}
	};

	std::future<void> AssetsTaskHandler::operator()(Concepts::Invokable<void()> auto&& func) const {
		std::lock_guard guard{this->lock};
		std::packaged_task<void()>& t = target->postedTasks.emplace(std::forward<decltype(func)>(func));
		return t.get_future();
	}

}