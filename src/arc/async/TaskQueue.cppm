//
// Created by Matrix on 2024/6/8.
//

export module Core.Async.TaskQueue;
import std;
import ext.Concepts;

export namespace Core::Async{
	template <typename FuncTy = void()>
	class TaskQueue{
	public:
		using TaskTy = std::packaged_task<FuncTy>;
		using RstTy = typename Concepts::FunctionTraits<FuncTy>::ReturnType;

	private:
		std::queue<std::packaged_task<FuncTy>> tasks{};
		mutable std::mutex mtx{};

	public:

		template <Concepts::Invokable<FuncTy> Func>
		[[nodiscard]] std::future<RstTy> push(Func&& task){
			return this->push(std::packaged_task<FuncTy>(std::forward<Func>(task)));
		}

		[[nodiscard]] std::future<RstTy> push(TaskTy&& task){
			std::future<RstTy> future = task.get_future();

			{
				std::scoped_lock lk{mtx};
				tasks.push(std::move(task));
			}

			return future;
		}

		[[nodiscard]] std::optional<TaskTy> pop() noexcept{
			std::scoped_lock lk{mtx};

			if(tasks.empty())return std::nullopt;

			auto func = std::move(tasks.front());
			tasks.pop();
			return std::move(func);
		}

		[[nodiscard]] bool empty() const noexcept{
			std::scoped_lock lk{mtx};
			return tasks.empty();
		}

		[[nodiscard]] auto size() const noexcept{
			std::scoped_lock lk{mtx};
			return tasks.size();
		}

		explicit operator bool() const noexcept{
			return !empty();
		}
	};
}
