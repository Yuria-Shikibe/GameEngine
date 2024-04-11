module UI.HoverTableManager;

import UI.Root;

bool UI::HoverTableManager::obtainValid(const Elem* lastRequester, const float minHoverTime, const bool useStaticTime) const{
	if(lastRequester == this->lastRequester)return false;

	if(root->cursorInBoundTime > 0.0f){
		if(useStaticTime){
			return root->cursorStrandedTime > minHoverTime;
		}else{
			return root->cursorInBoundTime > minHoverTime;
		}
	}

	return false;
}

void UI::HoverTableManager::update(const float delta){
	for(int i = 0; i < nextPopCount; ++i){
		droppedTables.pop_back();
	}
	nextPopCount = 0;

	if(currentFocus && lastRequester){
		if((!lastRequester->isCursorInbound() && followCursor) || (!currentFocus->isInbound(cursorPos) && !followCursor))
			dropCurrent();
	}

	if(currentFocus){
		if(followCursor){
			updateCurrentPosition();
		}

		currentFocus->update(delta);
	}

	for (const auto& dropped : droppedTables){
		dropped->update(delta);
	}
}
