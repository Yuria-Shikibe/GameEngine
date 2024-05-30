//
// Created by Matrix on 2024/4/5.
//

export module OS.GlobalTaskQueue;

import std;
import ext.Concepts;

namespace OS{
	bool handlerValid = false;

	std::mutex lockTask{};
	std::mutex lockAsyncTask{};
	std::mutex lockAsyncPackTask{};
	std::vector<std::function<void()>> postTasks;
	std::vector<std::pair<std::function<void()>, std::promise<void>>> postAsyncTasks;
	std::vector<std::packaged_task<void()>> postAsyncPackedTasks;
}

export namespace OS{
	void activateHander() {handlerValid = true;}

	[[nodiscard]] bool isHanderValid() {return handlerValid;}

	void deactivateHander() {handlerValid = false;}

	void post(Concepts::Invokable<void()> auto&& func) {
		std::lock_guard guard{lockTask};
		postTasks.push_back(std::forward<decltype(func)>(func));
	}

	void postAsync(Concepts::Invokable<void()> auto&& func, std::promise<void>&& promise) {
		std::lock_guard guard{lockAsyncTask};
		postAsyncTasks.emplace_back(std::forward<decltype(func)>(func), std::move(promise));
	}

	[[nodiscard]] std::future<void> postAsync(Concepts::Invokable<void()> auto&& func) {
		lockAsyncPackTask.lock();
		std::packaged_task<void()>& task = postAsyncPackedTasks.emplace_back(std::forward<decltype(func)>(func));
		lockAsyncPackTask.unlock();
		return task.get_future();
	}

	void handleTasks(){

		{
			lockTask.lock();
			const decltype(postTasks) tempTask = std::move(postTasks);
			lockTask.unlock();

			for(auto&& task : tempTask){
				task();
			}
		}

		{
			lockAsyncPackTask.lock();
			decltype(postAsyncPackedTasks) tempTask = std::move(postAsyncPackedTasks);
			lockAsyncPackTask.unlock();

			for(auto&& task : tempTask){

				task();
			}
		}

		{
			lockAsyncTask.lock();
			decltype(postAsyncTasks) tempTask = std::move(postAsyncTasks);
			lockAsyncTask.unlock();

			for(auto& [task, promise] : tempTask){
				try{
					task();
				}catch(...){
					promise.set_exception(std::current_exception());
				}
				promise.set_value();
			}
		}
	}
}
