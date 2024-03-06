module;

export module OS.Handler;

import Async;
import OS;

export namespace OS {
	struct OSTaskHandler : ext::TaskHandler{
		void operator()(Concepts::Invokable<void()> auto&& func, std::promise<void>&& promise) const {
			::OS::postAsync(
				std::forward<decltype(func)>(func),
				std::forward<std::promise<void>>(promise)
			);
		}

		std::future<void> operator()(Concepts::Invokable<void()> auto&& func) const {
			return ::OS::postAsync(
				std::forward<decltype(func)>(func)
			);
		}

		explicit operator bool() const {
			return ::OS::getLoop();
		}
	};
}


