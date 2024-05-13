module Assets.Directories;

import Core;

void Assets::loadDir(){
	using namespace Dir;
	OS::FileTree& mainTree = Core::rootFileTree;

	assets = mainTree.findDir("assets");
	patch(assets);

	shader = assets.subFile("shader");
	patch(shader);

	texture = assets.subFile("texture");
	patch(texture);

	font = assets.subFile("fonts");
	patch(font);
	
	sound = assets.subFile("sounds");
	patch(sound);

	svg = assets.subFile("svg");
	patch(svg);

	bundle = assets.subFile("bundles");
	patch(bundle);

	texCache = assets.subFile("tex-cache");
	patch(texCache);
	//
	// screenshot = mainTree.find("screenshots"); //TODO move this to other places, it doesn't belong to assets!
	// patch(texCache);
}
