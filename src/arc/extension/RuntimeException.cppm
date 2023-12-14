//
// Created by Matrix on 2023/11/21.
//
module;

export module RuntimeException;

import StackTrace;
import <sstream>;
import <string>;
import <exception>;

export namespace ext{
	class RuntimeException : virtual public std::exception{
	public:
		std::string data{};

		explicit RuntimeException(const std::string& str){
			std::stringstream ss;

			ss << str << '\n';

			getStackTraceBrief(ss);

			data = ss.str();

			RuntimeException::postProcess();
		}

		[[nodiscard]] char const* what() const override {
			return data.data();
		}

		virtual void postProcess() const;

		RuntimeException() : RuntimeException("Crashed At...") {

		}
	};

	class NullPointerException final : RuntimeException{
	public:
		[[nodiscard]] explicit NullPointerException(const std::string& str)
			: RuntimeException(str) {
		}

		[[nodiscard]] NullPointerException() : NullPointerException("Null Pointer At...") {

		}
	};

	class IllegalArguments final : RuntimeException{
	public:
	[[nodiscard]] explicit IllegalArguments(const std::string& str)
		: RuntimeException(str) {
	}

	[[nodiscard]] IllegalArguments() : IllegalArguments("Illegal Arguments At...") {

	}
};
}