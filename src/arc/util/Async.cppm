module;

export module Async;

import Concepts;
import RuntimeException;
import std;

export namespace ext {
	/**
	 * \brief Uses this handler to post tasks to other threads, usually main thread, useful for GL functions which is main thread only
	 */
	struct TaskHandler {
		std::future<void> operator()(Concepts::Invokable<void()> auto&& func) const {
			std::packaged_task<void()> t{std::forward<decltype(func)>(func)};
			auto f = t.get_future();
			t();
			return f;
		}

		explicit operator bool() const {
			return true;
		}
	};

	template <typename T = void, Concepts::Derived<TaskHandler> Handler = TaskHandler>
	class Task {
	protected:
		Handler* handler{nullptr};
		bool deferred{false};
	public:
		virtual ~Task() = default;

		[[nodiscard]] virtual std::future<T> launch(const std::launch policy){
			if(policy == std::launch::deferred)deferred = true;
			return {};
		}

		[[nodiscard]] virtual std::future<T> launch(){
			return launch(std::launch::async);
		}

		[[nodiscard]] Handler* getHandler() const{ return handler; }

		void setHandler(Handler* const handler) {
			this->handler = handler;
		}

		[[nodiscard]] std::future<void> postToHandler(Concepts::Invokable<void()> auto&& func) {
			if(!deferred){
				return handler->operator()(std::forward<decltype(func)>(func));
			}else{
				return this->fallbackPost(std::forward<decltype(func)>(func));
			}
		}

		virtual std::future<void> fallbackPost(std::function<void()>&& func){
			constexpr TaskHandler handler{};
			return handler(std::forward<decltype(func)>(func));
		}
	};

	template <typename T = void, Concepts::Derived<TaskHandler> Handler = TaskHandler>
	class ProgressTask : public Task<T, Handler>{
	protected:
		float taskProgress = 0;
		bool done = false;

	public:
		~ProgressTask() override = default;

		[[nodiscard]] virtual std::string_view getTaskName() const = 0;

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

		void setProgress(const int totalWeight, const int currentPartWeight, const int currentPartOffset, const float progress) {
			taskProgress = std::clamp(
				static_cast<float>(currentPartWeight) / static_cast<float>(totalWeight) * progress +
				static_cast<float>(currentPartOffset) / static_cast<float>(totalWeight),
				0.0f, 1.0f);
		}

		void setProgress(const float f) {
			taskProgress = std::clamp(f, 0.0f, 1.0f);
		}

		void addProgress_onePart(const size_t totalPart) {
			taskProgress += 1 / static_cast<float>(totalPart);
		}

		void setDone() {
			taskProgress = 1.0f;
			done = true;
		}

		void resetProgress() {
			taskProgress = 0.0f;
		}

		[[nodiscard]] bool finished() const {
			return done;
		}
	};
}
