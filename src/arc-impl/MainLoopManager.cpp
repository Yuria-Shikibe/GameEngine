module Core.MainLoopManager;

import OS;

Core::Tick Core::MainLoopManager::getDeltaTick() noexcept{
	return OS::deltaTick();
}

Core::Tick Core::MainLoopManager::getUpdateDeltaTick() noexcept{
	return OS::updateDeltaTick();
}

bool Core::MainLoopManager::isPaused() noexcept{
	return OS::isPaused();
}

void Core::MainLoopManager::handleAsync(){
	OS::handleTasks();
}
