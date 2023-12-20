module;

#include <thread>

module OS;

import Core;
import RuntimeException;

void ext::RuntimeException::postProcess() const {
	if(Core::log) {
		Core::log->generateCrash(what(), "RUNTIME");
	}

	if(std::this_thread::get_id() != OS::getMainThreadID()) {
		OS::exitApplication(3, what());
	}
}
