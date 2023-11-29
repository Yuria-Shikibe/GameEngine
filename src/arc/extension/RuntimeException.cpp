//
// Created by Matrix on 2023/11/29.
//

import Core;
import RuntimeException;

void ext::RuntimeException::postToLog() const {
	if(Core::log) {
		Core::log->generateCrash(what());
	}
}
