module UI.HoverTableManager;

import UI.Root;

void UI::HoverTableManager::drop(std::unique_ptr<Table>&& element, const bool instantDrop){
	if(instantDrop){
		element.reset(nullptr);
	}else{
		element->pushAction<Actions::AlphaMaskAction>(12.0f, 0.0f);
		element->pushAction<Actions::RunnableAction<Widget, TableDeleter>>(deleter);

		if(root->currentCursorFocus == element.get()){
			root->determinShiftFocus(nullptr);
		}

		droppedTables.push_front(std::move(element));
	}
}

void UI::HoverTableManager::dropCurrentAt(const Widget* where, const bool instantDrop){
	if(focusTableStack.empty())return;

	lastRequester = nullptr;

	if(where == nullptr){
		for(auto& element : focusTableStack | std::ranges::views::elements<0> | std::ranges::views::reverse){
			drop(std::move(element), instantDrop);
		}

		focusTableStack.clear();

		lastRequester = getCurrentConsumer();
	}

	const auto begin =
		std::ranges::find(focusTableStack | std::ranges::views::elements<0>, where, &std::unique_ptr<Table>::get).base();
	const auto end = focusTableStack.end();

	for(auto&& element : std::ranges::subrange{begin, end} | std::ranges::views::elements<0> | std::ranges::views::reverse){
		drop(std::move(element), instantDrop);
	}

	focusTableStack.erase(begin, end);

	lastRequester = getCurrentConsumer();
}

bool UI::HoverTableManager::obtainValid(const Widget* lastRequester, const float minHoverTime, const bool useStaticTime) const{
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

	auto* current = getCurrentFocus();

	if(current && lastRequester){
		if((!lastRequester->isCursorInbound() && followCursor) || (!current->isInbound(cursorPos) && !followCursor))
			dropCurrentAt(current);
	}

	if(current){
		if(followCursor){
			updateCurrentPosition(current);
		}
	}

	for (const auto& dropped : focusTableStack | std::ranges::views::elements<0>){
		dropped->update(delta);
	}

	for (const auto& dropped : droppedTables){
		dropped->update(delta);
	}
}
