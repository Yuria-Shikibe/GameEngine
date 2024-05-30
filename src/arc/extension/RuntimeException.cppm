export module ext.RuntimeException;

import StackTrace;
import std;

export namespace ext{
	class RuntimeException : public std::exception{
	public:
		~RuntimeException() override = default;

		std::string data{};

		explicit RuntimeException(const std::string& str, const std::source_location& location = std::source_location::current()){
			std::ostringstream ss;

			ss << str << '\n';
			ss << "at: " << location.file_name() << '\n';
			ss << "at: " << location.function_name() << '\n';
			ss << "at: " << location.column() << '\n';

			ext::getStackTraceBrief(ss);

			data = std::move(ss).str();

			RuntimeException::postProcess();
		}


		[[nodiscard]] char const* what() const override {
			return data.data();
		}

		virtual void postProcess() const;

		RuntimeException() : RuntimeException("Crashed At...") {

		}
	};

	class NullPointerException final : public RuntimeException{
	public:
		[[nodiscard]] explicit NullPointerException(const std::string& str)
			: RuntimeException(str) {
		}

		[[nodiscard]] NullPointerException() : NullPointerException("Null Pointer At...") {

		}
	};

	class IllegalArguments final : public RuntimeException {
	public:
		[[nodiscard]] explicit IllegalArguments(const std::string& str)
			: RuntimeException(str) {
		}

		[[nodiscard]] IllegalArguments() : IllegalArguments("Illegal Arguments At...") {

		}
	};

	class ArrayIndexOutOfBound final : public RuntimeException {
	public:
		[[nodiscard]] explicit ArrayIndexOutOfBound(const std::string& str)
			: RuntimeException(str) {
		}

		[[nodiscard]] ArrayIndexOutOfBound(const size_t index, const size_t bound) : RuntimeException("Array Index Out Of Bound! : [" + std::to_string(index) + "] Out of " + std::to_string(bound)) {

		}
	};
}