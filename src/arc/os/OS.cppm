module;

#include <csignal>

export module OS;

import std;

namespace OS{
	[[noreturn]] void exitApplication(int s, std::string_view what);

	[[noreturn]] void exitApplication(const int s) {exitApplication(s, "");}

	void launchApplication(){ //TODO optm this
		std::signal(SIGABRT, exitApplication);
		std::signal(SIGILL, exitApplication);
		std::signal(SIGSEGV, exitApplication);
		std::signal(SIGINT, exitApplication);
		std::signal(SIGTERM, exitApplication);
	}

	int init{[]() {
		launchApplication();
		return 0;
	}()};
}


