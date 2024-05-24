module UI.Action.Actions;

import UI.Group;

void UI::Actions::RemoveAction::begin(Elem* elem){
	if(elem->getParent()){
		elem->getParent()->postRemove(elem);
	}
}
