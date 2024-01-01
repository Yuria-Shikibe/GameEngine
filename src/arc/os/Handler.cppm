module;

export module OS.Handler;

import Async;
import OS;

export namespace OS {
	struct OSTaskHandler : ext::TaskHandler{
		template <Concepts::Invokable<void()> Func>
		void operator()(Func&& task, std::promise<void>&& promise) const {

			::OS::postAsync(
				std::forward<Func>(task),
				std::forward<std::promise<void>>(promise)
			);
		}

		explicit operator bool() const {
			return ::OS::getLoop();
		}
	};
}


