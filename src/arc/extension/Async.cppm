//
// Created by Matrix on 2023/12/8.
//
module;

export module Async;

import <string>;

export namespace ext {
	class Task {
	public:
		virtual ~Task() = default;
		virtual void launch() = 0;
	};

	class ReadableTask : public Task {
		[[nodiscard]] virtual std::string getTaskName() const = 0;

		[[nodiscard]] virtual float getProgress() const {return 0;}
	};
}
