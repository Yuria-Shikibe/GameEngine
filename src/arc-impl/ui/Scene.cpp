module UI.Scene;

import UI.Root;
bool UI::Scene::getRootVisiable() const{
	return !root->isHidden && root->currentScene == this;
}
