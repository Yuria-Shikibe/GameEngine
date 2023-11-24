
//
// Created by Matrix on 2023/11/21.
//

//TODO Redirect cout, generate crash logs...

module ;

export module Core.Log;

import <utility>;
import <chrono>;
import <string>;
import <sstream>;
export import File;

export namespace Core{
	class Log{
		static constexpr std::string_view INFO = "[Info]-";
		static constexpr std::string_view CRASH = "[Crash]-";
		static constexpr unsigned int maxCrashCache = 10;
		static constexpr unsigned int maxLogCache = 10;

	protected:
		OS::File logDir;
		OS::File crahsDir;
		OS::File runtimeLogFile;

	public:
		[[nodiscard]] explicit Log(const OS::File& dir)
			: logDir(dir) {
			crahsDir = logDir.subFile("crashes");
			runtimeLogFile = logDir.subFile(static_cast<std::string>(INFO) + std::to_string(getCurrentSystemTime()));
		}

		static long long getCurrentSystemTime() {
			return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		}

		OS::File generateCrashFile(const std::string& suffix = std::to_string(getCurrentSystemTime())) const{
			const OS::File crash = crahsDir.subFile(static_cast<std::string>(CRASH) + suffix);

			if(auto&& subs = crahsDir.subs(); subs.size() > maxCrashCache) {
				const auto& file = subs.back();
				subs.pop_back();

				file.deleteFile();
			}

			// ReSharper disable once CppExpressionWithoutSideEffects
			crash.createFile();
			return crash;
		}

		void generateCrash(const std::string& what) const{
			const OS::File crash = generateCrashFile();
			crash.writeString(what);
		}
	};
}
