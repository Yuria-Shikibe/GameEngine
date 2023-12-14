//
// Created by Matrix on 2023/11/29.
//

#include <thread>
import Core;
import OS;
import RuntimeException;

void ext::RuntimeException::postProcess() const {
	if(Core::log) {
		Core::log->generateCrash(what(), "RUNTIME");
	}

	if(std::this_thread::get_id() != OS::getMainThreadID()) {
		std::terminate();
	}
}
