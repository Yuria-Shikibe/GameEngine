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


	cache = mainTree.findDir("resource").subFile("cache");
	patch(cache);

	texCache = cache.subFile("tex");
	patch(texCache);

	game = assets.subFile("game");
	patch(game);

	data = mainTree.findDir("resource").subFile("data");
	patch(data);

	settings = data.subFile("settings");
	patch(settings);
	//
	// screenshot = mainTree.find("screenshots"); //TODO move this to other places, it doesn't belong to assets!
	// patch(texCache);
}
