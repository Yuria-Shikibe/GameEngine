module;

export module Async;

export import <future>;
import OS;
import Concepts;
import <string>;
import <functional>;

export namespace ext {
	struct TaskHandler {
		template <Concepts::Invokable<void()> Func>
		void operator()(Func&& task, std::promise<void>&& promise) const {
			task();
			promise.set_value();
		}
	};

	struct TaskHandler_PostMain : TaskHandler{
		template <Concepts::Invokable<void()> Func>
		void operator()(Func&& task, std::promise<void>&& promise) const {
			OS::postAsync(
				std::forward<Func>(task),
				std::forward<std::promise<void>>(promise)
			);
		}
	};

	template <typename T = void, Concepts::Derived<TaskHandler> Handler = TaskHandler>
	class Task {
	protected:
		Handler handler{};
	public:
		virtual ~Task() = default;

		[[nodiscard]] virtual std::future<T> launch() = 0;

		template <Concepts::Invokable<void()> Func>
		std::future<void> postToHandler(Func&& function) {
			std::promise<void> promise{};
			std::future<void>&& future = promise.get_future();
			handler(std::forward<Func>(function), std::move(promise));
			return future;
		}
	};

	template <typename T = void, Concepts::Derived<TaskHandler> Handler = TaskHandler_PostMain>
	class ProgressTask : public Task<T, Handler>{
	protected:
		float taskProgress = 0;

	public:
		~ProgressTask() override = default;

		[[nodiscard]] virtual std::string getTaskName() const = 0;

		[[nodiscard]] float getProgress() const {
			return taskProgress;
		}

		void setProgress(const int totalWeight, const int currentPartWeight, const int currentPartOffset, const int totalProgress = 1, const int currentProgress = 1) {
			taskProgress = std::clamp(
				static_cast<float>(currentPartWeight) / static_cast<float>(totalWeight) *
				static_cast<float>(currentProgress) / static_cast<float>(totalProgress) +
				static_cast<float>(currentPartOffset) / static_cast<float>(totalWeight),
				0.0f, 1.0f);
		}

		void setProgress(const float f) {
			taskProgress = std::clamp(f, 0.0f, 1.0f);
		}

		void setDone() {
			taskProgress = 1.0f;
		}

		void resetProgress() {
			taskProgress = 0.0f;
		}
	};
}
