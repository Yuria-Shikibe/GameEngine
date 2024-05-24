module UI.TooltipManager;

import UI.Root;

void UI::TooltipManager::drop(std::unique_ptr<Table>&& element, const bool instantDrop){
	if(instantDrop){
		element.reset(nullptr);
	}else{
		element->pushAction<Actions::AlphaMaskAction>(12.0f, 0.0f);
		element->pushAction<Actions::RunnableAction<Elem, TableDeleter>>(deleter);

		if(root->currentCursorFocus == element.get()){
			root->determinShiftFocus(nullptr);
		}

		droppedTables.push_front(std::move(element));
	}
}

void UI::TooltipManager::dropCurrentAt(const Elem* where, const bool instantDrop){
	if(focusTableStack.empty())return;

	lastConsumer = nullptr;

	if(where == nullptr){
		for(auto& element : focusTableStack | std::ranges::views::elements<0> | std::ranges::views::reverse){
			drop(std::move(element), instantDrop);
		}

		focusTableStack.clear();

		lastConsumer = getCurrentConsumer();
	}

	const auto begin =
		std::ranges::find(focusTableStack | std::ranges::views::elements<0>, where, &std::unique_ptr<Table>::get).base();
	const auto end = focusTableStack.end();

	for(auto&& element : std::ranges::subrange{begin, end} | std::ranges::views::elements<0> | std::ranges::views::reverse){
		drop(std::move(element), instantDrop);
	}

	focusTableStack.erase(begin, end);

	lastConsumer = getCurrentConsumer();
}

UI::Table* UI::TooltipManager::tryObtain(const Elem* consumer){
	if(consumer && consumer->getTooltipBuilder() &&
		obtainValid(
			consumer,
			consumer->getTooltipBuilder().minHoverTime,
			consumer->getTooltipBuilder().useStaticTime)
	){
		root->cursorStrandedTime = root->cursorInBoundTime = 0;

		return generateTooltip(consumer->getTooltipBuilder().builder, consumer);
	}

	return nullptr;
}

bool UI::TooltipManager::obtainValid(const Elem* lastRequester, const float minHoverTime, const bool useStaticTime) const{
	if(lastRequester == this->lastConsumer)return false;

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
	for(unsigned i = 0; i < nextPopCount; ++i){
		droppedTables.pop_back();
	}
	nextPopCount = 0;

	auto* current = getCurrentFocus();
	const bool autoRelease = lastConsumer && lastConsumer->getTooltipBuilder().autoRelease;

	if(current && lastConsumer){
		const auto& builder = lastConsumer->getTooltipBuilder();

		if(builder.followTarget != TooltipFollowTarget::none)updateCurrentPosition(current);

		if(autoRelease){
			switch(builder.followTarget){
				case TooltipFollowTarget::cursor:{
					if(!lastConsumer->isCursorInbound())
						dropCurrentAt(current);
					break;
				}

				case TooltipFollowTarget::none:{
					if(!current->isInbound(cursorPos))
						dropCurrentAt(current);
					break;
				}

				case TooltipFollowTarget::parent:{
					if(!current->isInbound(cursorPos) && !lastConsumer->isInbound(cursorPos))
						dropCurrentAt(current);
					break;
				}

				default: dropCurrentAt(current);
			}
		}

	}

	for (const auto& dropped : focusTableStack | std::ranges::views::elements<0>){
		dropped->update(delta);
	}

	for (const auto& dropped : droppedTables){
		dropped->update(delta);
	}
}
