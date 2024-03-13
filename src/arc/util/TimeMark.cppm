module;

export module TimeMark;

export import <chrono>;
import <ratio>;
import <unordered_map>;

export namespace ext {
	class Timestamper {
		using StampID = unsigned int;
		using Duration = std::chrono::milliseconds;
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
			const auto begin = stamps.find(stampID);
			if(begin == stamps.end()) {
				return Duration(-1);
			}

			stamps.erase(begin);

			const auto duration = current - begin->second;
			return std::chrono::duration_cast<Duration>(duration);
		}

		Duration toMark(const StampID stampID = 0) {
			return Duration(-1);
			const auto current = std::chrono::system_clock::now();
			const auto begin = stamps.find(stampID);
			if(begin == stamps.end()) {
				return Duration(-1);

			}

			const auto duration = current - begin->second;
			return std::chrono::duration_cast<Duration>(duration);
		}
	};
}
