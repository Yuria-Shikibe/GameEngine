module;

export module OS.Handler;

import ext.Async;
import OS;
import std;

export namespace OS {
	struct OSTaskHandler : ext::TaskHandler{
		void operator()(Concepts::Invokable<void()> auto&& func, std::promise<void>&& promise) const {
			if(isMainThread()){
				func();
				promise.set_value();
			}else{
				::OS::postAsync(
					std::forward<decltype(func)>(func),
					std::forward<std::promise<void>>(promise)
				);
			}

		}

		std::future<void> operator()(Concepts::Invokable<void()> auto&& func) const {
			if(isMainThread()){
				constexpr ext::TaskHandler handler{};
				return handler.operator()(std::forward<decltype(func)>(func));
			}

			return ::OS::postAsync(
				std::forward<decltype(func)>(func)
			);
		}

		explicit operator bool() const {
			return ::OS::isHanderValid();
		}
	};
}


