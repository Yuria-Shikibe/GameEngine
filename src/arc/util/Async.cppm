module;

export module Async;

export import <future>;
import Concepts;
import <string>;
import <functional>;

export namespace ext {
	/**
	 * \brief Uses this handler to post tasks to other threads, usually main thread, useful for GL functions which is main thread only
	 */
	struct TaskHandler {
		template <Concepts::Invokable<void()> Func>
		void operator()(Func&& task, std::promise<void>&& promise) const {
			task();
			promise.set_value();
		}

		explicit operator bool() const {
			return true;
		}
	};

	template <typename T = void, Concepts::Derived<TaskHandler> Handler = TaskHandler>
	class Task {
	protected:
		Handler* handler{nullptr};
	public:
		virtual ~Task() = default;

		[[nodiscard]] virtual std::future<T> launch(std::launch policy) = 0;

		[[nodiscard]] virtual std::future<T> launch(){
			return launch(std::launch::async);
		}

		void setHandler(Handler* const handler) {
			this->handler = handler;
		}

		template <Concepts::Invokable<void()> Func>
		[[nodiscard]] std::future<void> postToHandler(Func&& function) {

			std::promise<void> promise{};
			std::future<void>&& future = promise.get_future();
			if(*handler)handler->operator()(std::forward<Func>(function), std::move(promise));
			else { //TODO may add exception throw?
				function();
				promise.set_value();
			}
			return future;
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

	template <Concepts::Derived<TaskHandler> Handler, typename Func>
	class FuncTask : public ProgressTask<std::invoke_result_t<Func>, Handler> {
		using T = std::invoke_result_t<Func>;
		Func func;


	public:
		[[nodiscard]] FuncTask() = default;

		[[nodiscard]] explicit FuncTask(Func& func)
			: func(func) {
		}

		[[nodiscard]] explicit FuncTask(Func&& func)
			: func(std::forward<Func>(func)) {
		}

		template<typename _T = T> requires std::same_as<_T, void>
		void run() {
			func();
			ProgressTask<T, Handler>::setDone();
		}

		T run() {
			T ret = func();
			ProgressTask<T, Handler>::setDone();
			return ret;
		}

		std::future<T> launch(std::launch policy) override {
			return std::async(policy, &FuncTask::run<T>, this);
		}

		[[nodiscard]] std::string_view getTaskName() const override {
			return "Anonymous Function Running...";
		}
	};

	template <Concepts::Derived<TaskHandler> Handler, typename Func>
	[[nodiscard]] FuncTask<Handler, Func>create(Func&& func) {
		return ext::FuncTask<Handler, Func>{func};
	}

}
