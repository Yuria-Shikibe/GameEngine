module;

export module TimeMark;

export import <chrono>;
export import <ratio>;
import <unordered_map>;

export namespace ext {
	class Timestamper {
		using StampID = unsigned int;
		using StdUnit = std::chrono::milliseconds;
		using Duration = std::chrono::duration<long long, std::milli>;
		std::unordered_map<StampID, std::chrono::time_point<std::chrono::system_clock>> stamps{};

	public:
		[[nodiscard]] size_t size() const {
			return stamps.size();
		}

		void mark(const StampID stampID = 0) {
			stamps.insert_or_assign(stampID, std::chrono::system_clock::now());
		}

		Duration popMark(const StampID stampID = 0) {
			const auto current = std::chrono::system_clock::now();
			auto&& begin = stamps.find(stampID);
			if(begin == stamps.end()) {
				return Duration(-1);
			}

			stamps.erase(begin);

			return std::chrono::duration_cast<StdUnit>(current - begin->second);
		}

		Duration toMark(const StampID stampID = 0) {
			const auto current = std::chrono::system_clock::now();
			auto&& begin = stamps.find(stampID);
			if(begin == stamps.end()) {
				return Duration(-1);
			}

			return std::chrono::duration_cast<StdUnit>(current - begin->second);
		}
	};
}
