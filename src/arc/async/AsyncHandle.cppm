//
// Created by Matrix on 2024/6/8.
//

export module Core.Async.Handle;

import std;

export namespace Core::Async{
	struct Handle{
		[[nodiscard]] std::future<void> postTask(std::packaged_task<void()>&& task) const{
			task.operator()();
			return task.get_future();
		}

		void throwException(const std::exception_ptr& postException) const noexcept{
			std::rethrow_exception(postException);
		}

		explicit operator bool() const noexcept{return true;}
	};
}
