//
// Created by Matrix on 2024/5/13.
//

export module Assets.Directories;

export import OS.File;
export import OS.FileTree;

export namespace Assets{
	namespace Dir{
		inline OS::File assets;
		inline OS::File shader;
		inline OS::File texture;
		inline OS::File texCache;
		inline OS::File font;
		inline OS::File svg;
		inline OS::File bundle;
		inline OS::File sound;

		inline OS::File data;
		inline OS::File settings;

		inline OS::File game;
	}


	void patch(const OS::File& file){
		if(!file.exist()){
			file.createDirQuiet();
		}
	}

	void loadDir();
}
