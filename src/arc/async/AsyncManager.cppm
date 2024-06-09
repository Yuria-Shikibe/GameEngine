//
// Created by Matrix on 2024/6/8.
//

export module Core.Async.Manager;

import Core.Async.TaskQueue;
import std;

export namespace Core::Async{
	struct AsyncTask{
		virtual ~AsyncTask() = default;

		[[nodiscard]] virtual std::future<void> launch(std::launch policy) = 0;

		[[nodiscard]] std::future<void> launch(){
			return launch(std::launch::async);
		}

		[[nodiscard]] virtual float getProgress() const noexcept = 0;

		[[nodiscard]] virtual std::string_view getTaskName() const noexcept {return "";}


	};
	struct AsyncHandler;
	class AsyncManager{
		TaskQueue<void()> taskQueue{};
		std::vector<AsyncHandler*> handlers{};

		friend AsyncManager;
		// void
	};

}
