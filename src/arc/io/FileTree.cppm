module ;

export module OS.FileTree;

import Platform;
import File;
import RuntimeException;
import <iostream>;
import <unordered_map>;
import <string>;
import <span>;
import <vector>;
import <ostream>;
import <ranges>;
import <sstream>;

export namespace OS{
	//dTODO use a struct to wrap the data structure of just directly use vector?

	/**
	 * \brief Uses to operate frequently used `readonly` files, static!
	 * */
	class FileTree{
	protected:
		/**
		 * \brief empty for add all subs
		 * */
		std::vector<File::sortPred> concentration{};

		std::vector<File> subDirectories{};

		/**
		 * \brief Key - directory name | Value - File
		 * */
		std::unordered_map<std::string, std::vector<File>> files{};


		/**
		 * \brief This need to be called to enable it.
		 */
		std::unordered_map<std::string, File> mappedFiles{};

		File root{};

	public:
		[[nodiscard]] bool concentrateAll() const{
			return concentration.empty();
		}

		FileTree() : root{ext::getSelf_runTime()} {
			buildFileTree();
		}

		explicit FileTree(const std::string& root) : root{root} {
			buildFileTree();
		}

		explicit FileTree(const File& root) : root{root} {
			buildFileTree();
		}

		explicit FileTree(File&& root) : root{std::move(root)} {
			buildFileTree();
		}

		FileTree(const FileTree& o){
			reDirect(o.root);
			concentration = o.concentration;
			buildFileTree();
		}

		FileTree(FileTree&& o) noexcept {
			reDirect(o.root);
			concentration = std::move(o.concentration);
			subDirectories = std::move(o.subDirectories);
			files = std::move(o.files);
		}

		FileTree& operator=(const FileTree& o){
			if(this == &o)return *this;
			reDirect(o.root);
			concentration = o.concentration;
			subDirectories = o.subDirectories;
			buildFileTree();

			return *this;
		}

		FileTree& operator=(FileTree&& o) noexcept {
			if(this == &o)return *this;
			reDirect(o.root);
			concentration = std::move(o.concentration);
			subDirectories = std::move(o.subDirectories);
			files = std::move(o.files);

			return *this;
		}

		static void output(std::ostream &os, const File& file, const unsigned char depth){
			if(depth == 0){
				os << file.path().parent_path().string() << "/\n";
			}

			for (int i = 0; i < depth; ++i) {
				os << "  |\t";
			}

			os << file << '\n';

			if(file.isDir()){
				file.forSubs([d = depth + 1, &os](File&& f){
					output(os, f, d);
				});
			}
		}

		friend std::ostream &operator<<(std::ostream &os, const FileTree& tree) {
			output(os, tree.root, 0);

			os << std::endl;

			return os;
		}

		[[nodiscard]] const std::unordered_map<std::string, std::vector<File>>& getFiles() const{
			return files;
		}

		[[nodiscard]] const std::vector<File>& getDirs() const{
			return subDirectories;
		}

		template<typename ...T>
			requires (std::is_same_v<T, File::sortPred> && ...)
		void registerConcentration(T... args){
			(concentration.push_back(args), ...);
		}

		/**
		 * \brief Avoid using this if posssible!
		 * \param prefix
		 */
		void mapSubFiles(const std::string& prefix = std::string{}) {
			for(auto& element : files | std::ranges::views::values) {
				for(auto& file : element) {
					if(const auto [itr, success] = mappedFiles.emplace(prefix + file.stem(), file); !success) {
						throw ext::RuntimeException{"It's illegal to map file tree that has files with the same stem name!"};
					}
				}
			}
		}

		void reDirect(const File& rootFile){
			root = rootFile;

			files.clear();
		}

		void buildFileTree(const bool clearLegacy = false){
			if(clearLegacy)files.clear();

			if(concentrateAll()){
				files = std::move(root.sortSubs(true));
			}else{
				files = std::move(root.sortSubsBy(concentration, true));
			}

			for (const auto noExtensions = files[static_cast<std::string>(File::EMPTY_EXTENSION)]; const auto& file : noExtensions) {
				if(file.isDir())subDirectories.push_back(file);
			}
		}

		File findDir(const std::string& name){
			const auto it = std::ranges::find_if(subDirectories, [&name](const File& o){
				return o.filename() == name;
			});

			if(it == subDirectories.end()){
				return root;
			}

			return *it;
		}

		[[nodiscard]] const std::vector<File>& function(const std::string& sortName) const{
			return files.at(sortName);
		}

		/**
		 * \brief Return root if find nothing!
		 * */
		[[nodiscard]] File find(const std::string& fileName, const std::string& sortName = ""){
			if(sortName.empty()){
				auto values = files | std::ranges::views::values;

				std::vector<File>::iterator index2;

				if(const auto index1 = std::ranges::find_if(values, [&fileName, &index2](std::vector<File>& fileVector) mutable {
					index2 = std::ranges::find_if(fileVector, [&fileName](const File& file){
						return fileName == file.filename();
					});

					return index2 != fileVector.end();
				}); index1 != values.end() && index2 != (*index1).end()){
					return *index2;
				}

				return root;
			}

			if(!files.contains(sortName))return root;
			auto sameTypes = files.at(sortName);

			if(const auto result = std::ranges::find_if(sameTypes, [&fileName](const File& file){
				return fileName == file.filename();
			}); result != sameTypes.end()){
				return *result;
			}
			return root;
		}
	};
}