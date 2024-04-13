module UI.Action.Actions;

import UI.Group;

void UI::Actions::RemoveAction::begin(Widget* elem){
	if(elem->getParent()){
		elem->getParent()->postRemove(elem);
	}
}
