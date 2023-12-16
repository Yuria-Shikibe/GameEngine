//
// Created by Matrix on 2023/11/22.
//
module ;

export module Assets.Loader;

import Concepts;
import Async;
import TimeMark;
import RuntimeException;
import <sstream>;
import <numeric>;
import <queue>;
import <stack>;
import <ranges>;
import <vector>;

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

		void operator()(std::function<void()>&& task, std::promise<void>&& promise) const;
	};

	class AssetsLoader{
		using TaskFuture = std::future<void>;
		using Handler = AssetsTaskHandler;
		using Task = ext::ProgressTask<void, Handler>*;

		static constexpr auto maxLoadSpacing = std::chrono::duration<long long, std::milli>(330);

		ext::Timestamper timer{};
		std::unordered_map<Task, TaskFuture> tasks{};

		std::unique_ptr<Handler> postHandler{nullptr};

		mutable std::stringstream sstream{};

		float taskProgress{0};
		bool done{false};

	public:
		[[nodiscard]] AssetsLoader() : postHandler(std::make_unique<Handler>(this)) {

		}

		~AssetsLoader()  = default;

		std::queue<std::pair<std::function<void()>, std::promise<void>>> postedTasks{};

		void begin() {
			for(auto& [task, future] : tasks) {
				future = task->launch();
			}
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
		void processRequests() {
			timer.mark();

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

			auto duration = timer.toMark();

			while(duration < maxLoadSpacing) {
				this->postHandler->lock.lock();

				if(postedTasks.empty()) {
					this->postHandler->lock.unlock();
					break;
				}
				auto [task, promise] = std::move(postedTasks.front());
				postedTasks.pop();

				this->postHandler->lock.unlock();

				if(!task) {
					throw ext::RuntimeException{"Illegally Post Empty Functions!"};
				}

				try {
					task();
				}catch(...){throw std::current_exception();}

				promise.set_value();

				duration = timer.toMark();
			}
		}

		void forceGet() {
			while(!finished()) {
				processRequests();
			}
		}

		void push(Task const task) {
			task->setHandler(this->postHandler.get());
			tasks.try_emplace(task, TaskFuture{});
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
}