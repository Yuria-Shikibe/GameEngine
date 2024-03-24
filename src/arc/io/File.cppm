module ;

export module OS.File;

import Concepts;

import RuntimeException;

import std;

import Heterogeneous;

using namespace std::filesystem;

export namespace OS{
	class File {
	protected:
		std::filesystem::path rawPath{};

	public:
		static constexpr std::string_view EMPTY_EXTENSION = "[empty]";

		File() = default;

		explicit File(const decltype(rawPath)::string_type& p) : rawPath{p}{

		}

		explicit File(decltype(rawPath)::string_type&& p) : rawPath{std::move(p)}{

		}

		explicit File(const path& p) : rawPath{p}{

		}

		explicit File(path&& p) : rawPath{std::move(p)}{

		}

		explicit File(const directory_entry& p) : rawPath(p){

		}

		File& operator=(const path& other) {
			rawPath = other;
			return *this;
		}

		File& operator=(path&& other) {
			rawPath = std::move(other);
			return *this;
		}

		explicit operator directory_entry() const{
			return directory_entry{rawPath};
		}

		explicit operator path() const{
			return rawPath;
		}

		friend bool operator==(const File& lhs, const File& rhs){
			return lhs.rawPath == rhs.rawPath;
		}

		friend bool operator!=(const File& lhs, const File& rhs){
			return !(lhs == rhs);
		}

		[[nodiscard]] path absolutePath() const{
			return absolute(this->getPath());
		}

		[[nodiscard]] std::string extension() const {
			return rawPath.extension().string();
		}

		[[nodiscard]] std::string stem() const {
			return rawPath.stem().string();
		}

		[[nodiscard]] std::string filename() const {
			return rawPath.filename().string();
		}

		[[nodiscard]] std::string filename_full() const {
			return (isDir() ? "<Dir> " : "<Fi> ") + rawPath.filename().string();
		}

		[[nodiscard]] path& getPath() {
			return rawPath;
		}

		[[nodiscard]] const std::filesystem::path& getPath() const {
			return rawPath;
		}

		[[nodiscard]] bool exist() const {
			return std::filesystem::exists(rawPath);
		}

		[[maybe_unused]] bool deleteFile() const {  // NOLINT(*-use-nodiscard)
			return exist() && (isDir() ? remove(getPath()) : std::filesystem::remove_all(getPath()));
		}

		[[maybe_unused]] void deleteFileQuiet() const {  // NOLINT(*-use-nodiscard)
			isDir() ? remove(getPath()) : std::filesystem::remove_all(getPath());
		}

		[[nodiscard]] bool copy(const std::filesystem::path& dest) const {
			try {
				std::filesystem::copy(getPath(), dest);
				return true;
			}
			catch ([[maybe_unused]] std::error_code& ignore) {
				return false;
			}
		}

		[[nodiscard]] bool copy(const File& dest) const {
			return copy(dest.getPath());
		}

		[[nodiscard]] bool isDir() const {
			return is_directory(getPath());
		}

		[[nodiscard]] bool isRegular() const {
			return is_regular_file(getPath());
		}

		[[nodiscard]] bool isHidden() const {
			return stem().starts_with('.');
		}

		[[nodiscard]] bool emptyExtension() const {
			return extension().empty();
		}

		[[maybe_unused]] bool createDir(const bool autoCreateParents = true) const { // NOLINT(*-use-nodiscard)
			return  autoCreateParents ? create_directories(getPath()) : create_directory(getPath());
		}

		void createDirQuiet(const bool autoCreateParents = true) const { // NOLINT(*-use-nodiscard)
			// ReSharper disable once CppExpressionWithoutSideEffects
			createDir(autoCreateParents);
		}

		[[maybe_unused]] bool createFile(const bool autoCreateParents = false) const { // NOLINT(*-use-nodiscard)
			if (exist())return false;

			if(autoCreateParents){
				if(const File parent = File{getParent()}; !parent.exist()){
					return parent.createDir();
				}
			}

			//TODO shit
			std::ofstream ofStream(getPath());
			const bool valid = ofStream.is_open();
			ofStream.close();

			return valid;
		}

		void createFileQuiet(const bool autoCreateParents = false) const {
			// ReSharper disable once CppExpressionWithoutSideEffects
			createFile(autoCreateParents);
		}

		[[nodiscard]] File getParent() const {
			return File{ getPath().parent_path() };
		}

		[[nodiscard]] File subFile(const std::string_view name) const {
			if(!isDir())throw ext::RuntimeException{};

			return File{ absolutePath().append(name) };
		}

		[[nodiscard]] File subFile(const char* name) const {
			if(!isDir())throw ext::RuntimeException{};

			return File{ absolutePath().append(name) };
		}

		[[nodiscard]] File find(const std::string_view name) const{
			for (const auto & item : directory_iterator(getPath()))
			{
				if (item.path().filename() == name)
				{
					return File{item};
				}
			}

			throw ext::RuntimeException{"Unable To Find File: " + subFile(name).absolutePath().string()};
		}

		/**
		 * Warning: This does not do append but erase and write!
		 * */
		void writeString(const std::string& data, const bool autoFlush = true) const {
			if (std::ofstream ofStream(getPath()); ofStream.is_open()) {
				ofStream << data;

				if(autoFlush)ofStream.flush();
			}
		}

		[[nodiscard]] std::vector<File> subs(const bool careDirs = false) const{
			std::vector<File> files;
			for (const auto& item : directory_iterator(getPath())){
				if(item.is_directory()) {
					if(careDirs)files.emplace_back(item);
				}else {
					files.emplace_back(item);
				}
			}

			return files;
		}

		void forSubs(Concepts::Invokable<void(File&&)> auto&& consumer) const{
			for (const auto& item : directory_iterator(getPath())){
				consumer(File{item});
			}
		}

		void forAllSubs(Concepts::Invokable<void(File&&)> auto&& consumer, const bool careDirs = false) const{
			for (const auto& item : directory_iterator(getPath())){
				if(File f{item}; f.isRegular()){
					consumer(std::move(f));
				}else{
					f.forAllSubs(consumer, careDirs);
					if(careDirs) {
						consumer(std::move(f));
					}
				}
			}
		}

		void allSubs(std::vector<File>& container, const bool careDirs = false) const{
			for (const auto& item : directory_iterator(getPath())){
				if(File f{item}; f.isRegular()){
					container.emplace_back(std::move(f));
				}else{
					f.allSubs(container, careDirs);
					if(careDirs) {
						container.emplace_back(std::move(f));
					}
				}
			}
		}

		[[nodiscard]] std::string readString(Concepts::Invokable<void(std::string&)> auto&& consumer = nullptr) const {
			std::ifstream file_stream(getPath());

			if(!file_stream.is_open())return "";

			std::stringstream file_contents;
			std::string line;

			while (std::getline(file_stream, line)) {
				consumer(line);
				file_contents << line;
				if(!file_stream.eof()){
					file_contents << '\n';
				}
			}

			return file_contents.str();
		}

		void writeByte(Concepts::Invokable<void(const std::ofstream&)> auto&& func) {
			if(std::ofstream ofStream(absolutePath(), std::ios::binary); ofStream.is_open()) {
				func(ofStream);
			}
		}

		[[nodiscard]] std::string readString() const {
			std::ifstream file_stream(getPath());

			if(!file_stream.is_open())return "";

			std::stringstream file_contents;
			std::string line;

			while (std::getline(file_stream, line)) {
				file_contents << line;
				if(!file_stream.eof()){
					file_contents << '\n';
				}
			}

			return file_contents.str();
		}

		friend std::ostream &operator<<(std::ostream &os, const File &file) {
			os << file.filename_full();
			return os;
		}

		using Filter = std::pair<std::string, std::function<bool(const File&)>>;

		[[nodiscard]] ext::StringMap<std::vector<File>> sortSubs(const bool careDirs = false) const{
			ext::StringMap<std::vector<File>> map;
			forAllSubs([&map](File&& file){
				const std::string& extension = file.extension();
				map[extension.empty() ? static_cast<std::string>(EMPTY_EXTENSION) : extension].push_back(file);
			}, careDirs);

			return map;
		}

		[[nodiscard]] ext::StringMap<std::vector<File>> sortSubsBy(const std::span<Filter>& standards, const bool careDirs = false) const{
			ext::StringMap<std::vector<File>> map;

			forAllSubs([&standards, &map](File&& file){
				if (const auto it = std::ranges::find_if(standards, [&file](const Filter& pair){
					return pair.second(file);
				}); it != standards.end()) {
					map[it->first].push_back(file);
				}
			}, careDirs);

			return map;
		}

		static std::unordered_map<std::string, std::vector<File>> sortBy(const std::span<File>& files, const std::span<Filter>& standards){
			std::unordered_map<std::string, std::vector<File>> map;

			for (const File& file : files) {
				if (
					auto it = std::ranges::find_if(standards, [&file](const Filter& pair){
						return pair.second(file);
					});
					it != standards.end()
				) {
					map[it->first].push_back(file);
				}
			}

			return map;
		}
	};
}