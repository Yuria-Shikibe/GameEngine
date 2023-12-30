import Assets.Loader;
import <functional>;
import <future>;


void Assets::AssetsTaskHandler::operator()(std::function<void()>&& task, std::promise<void>&& promise) const {
	std::lock_guard guard{this->lock};

	target->postedTasks.emplace(
		std::forward<std::function<void()>>(task),
		std::forward<std::promise<void>>(promise)
	);
}

