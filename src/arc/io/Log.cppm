
//TODO Redirect cout, generate crash logs...

module ;


export module Core.Log;

import <utility>;

import <chrono>;
import <string>;
import <sstream>;
import <fstream>;
import <utility>;
export import File;
import <iostream>;

export namespace Core{
	class Log{
		static constexpr std::string_view INFO = "[Info]-";
		static constexpr std::string_view CRASH = "[Crash]-";
		static constexpr unsigned int maxCrashCache = 10;
		static constexpr unsigned int maxLogCache = 10;

	protected:
		OS::File logDir{};
		OS::File crahsDir{};

		OS::File logFile{};

		mutable std::ofstream logOfStream{};

	public:
		//TODO redirect cout
		[[nodiscard]] explicit Log(OS::File dir)
			: logDir(std::move(dir)) {

			crahsDir = logDir.subFile("crashes");
			crahsDir.createDirQuiet();

#ifndef DEBUG_LOCAL
			logOfStream.open(logFile.absolutePath(), std::ios::out);
			logFile = logDir.subFile(static_cast<std::string>(INFO) + getCurrentSystemTime());
			logFile.createFileQuiet();
			std::cout.rdbuf(logOfStream.rdbuf());
#endif
		}

		void log(const std::string_view message) {
#ifndef DEBUG_LOCAL
			logOfStream << message << std::endl; //Flush after message;
#endif
		}

		static std::string getCurrentSystemTime() {
			const auto systemTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			return std::to_string(systemTime);
		}

		[[nodiscard]] OS::File generateCrashFile(const std::string& stem = "", const std::string& suffix = getCurrentSystemTime()) const{
			const OS::File crash = crahsDir.subFile(static_cast<std::string>(CRASH) +  stem + "-" + suffix + ".txt");

			if(auto&& subs = crahsDir.subs(); subs.size() > maxCrashCache) {
				const auto& file = subs.back();
				subs.pop_back();

				file.deleteFileQuiet();
			}

			// ReSharper disable once CppExpressionWithoutSideEffects
			crash.createFile();
			return crash;
		}

		void generateCrash(const std::string& what, const std::string& type = "") const{
			const OS::File crash = generateCrashFile(type);
			crash.writeString(what);
		}
	};
}
