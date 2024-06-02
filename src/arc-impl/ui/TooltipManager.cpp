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

UI::Table* UI::TooltipManager::tryObtain(const Elem* consumer){
	if(consumer && consumer->getTooltipBuilder() &&
		obtainValid(
			consumer)
	){
		root->cursorStrandedTime = root->cursorInBoundTime = 0;

		return generateTooltip(consumer->getTooltipBuilder().builder, consumer);
	}

	return nullptr;
}

bool UI::TooltipManager::obtainValid(const Elem* consumer) const{
	if(consumer == this->lastConsumer)return false;

	auto& builder = consumer->getTooltipBuilder();

	if(root->tempTooltipBanned && builder.autoBuild())return false;

	if(root->cursorInBoundTime > 0.0f){
		if(builder.useStaticTime){
			return root->cursorStrandedTime > builder.minHoverTime;
		}else{
			return root->cursorInBoundTime > builder.minHoverTime;
		}
	}

	return false;
}

void UI::TooltipManager::update(const float delta){
	std::erase_if(droppedTables, [this](const decltype(droppedTables)::value_type& ptr){
		return toErase.contains(ptr.get());
	});

	toErase.clear();

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
