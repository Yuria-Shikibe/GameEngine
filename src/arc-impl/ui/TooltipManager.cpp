module UI.TooltipManager;

import UI.Root;

void UI::TooltipManager::drop(std::unique_ptr<Table>&& element, const bool instantDrop){
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

void UI::TooltipManager::dropCurrentAt(const Widget* where, const bool instantDrop){
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

UI::Table* UI::TooltipManager::tryObtain(const Widget* consumer){
	if(consumer && consumer->getTooltipBuilder() &&
		obtainValid(
			consumer,
			consumer->getTooltipBuilder().minHoverTime,
			consumer->getTooltipBuilder().useStaticTime)
	){
		root->cursorStrandedTime = root->cursorInBoundTime = 0;

		return obtainFromWedget(consumer);
	}

	return nullptr;
}

bool UI::TooltipManager::obtainValid(const Widget* lastRequester, const float minHoverTime, const bool useStaticTime) const{
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

void UI::TooltipManager::update(const float delta){
	for(int i = 0; i < nextPopCount; ++i){
		droppedTables.pop_back();
	}

	nextPopCount = 0;

	auto* current = getCurrentFocus();
	const bool autoRelease = lastRequester && lastRequester->getTooltipBuilder().autoRelease;
	const auto& builder = lastRequester->getTooltipBuilder();

	if(current && lastRequester){
		if(builder.followTarget != TooltipBuilder::FollowTarget::none)updateCurrentPosition(current);

		switch(builder.followTarget){
			case TooltipBuilder::FollowTarget::cursor:{
				if(autoRelease && !lastRequester->isCursorInbound())dropCurrentAt(current);
				break;
			}

			case TooltipBuilder::FollowTarget::none:{
				if(autoRelease && !current->isInbound(cursorPos))dropCurrentAt(current);
				break;
			}

			case TooltipBuilder::FollowTarget::parent:{
				if(autoRelease && !current->isInbound(cursorPos) && !lastRequester->isInbound(cursorPos))dropCurrentAt(current);
				break;
			}

			default: dropCurrentAt(current);
		}
	}

	for (const auto& dropped : focusTableStack | std::ranges::views::elements<0>){
		dropped->update(delta);
	}

	for (const auto& dropped : droppedTables){
		dropped->update(delta);
	}
}
