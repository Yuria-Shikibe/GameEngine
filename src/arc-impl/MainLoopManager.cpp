module Core.MainLoopManager;

import OS;

float Core::MainLoopManager::getDeltaTick() noexcept{
	return OS::deltaTick();
}

float Core::MainLoopManager::getUpdateDeltaTick() noexcept{
	return OS::updateDeltaTick();
}

bool Core::MainLoopManager::isPaused() noexcept{
	return OS::isPaused();
}

void Core::MainLoopManager::handleAsync(){
	OS::handleTasks();
}
