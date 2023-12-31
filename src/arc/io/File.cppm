module ;

export module File;

import Concepts;

import RuntimeException;

import <utility>;
import <unordered_map>;
import <span>;
import <memory>;
import <filesystem>;
import <fstream>;
import <vector>;
import <functional>;
import <string>;
import <execution>;

using namespace std::filesystem;

export namespace OS{
	class File {
	public:
		static constexpr std::string_view EMPTY_EXTENSION = "[empty]";

		path filePath{};

		File() = default;

		explicit File(const path& p) : filePath{p}{

		}

		explicit File(const directory_entry& p) : filePath(p){

		}

		explicit File(directory_entry&& p) : filePath(std::move(p)){

		}

		File(const File& other) = default;

		File(File&& other) noexcept
			: filePath(std::move(other.filePath)) {
		}

		File& operator=(const File& other) {
			if(this == &other) return *this;
			filePath = other.filePath;
			return *this;
		}

		File& operator=(File&& other) noexcept {
			if(this == &other) return *this;
			filePath = std::move(other.filePath);
			return *this;
		}

		explicit operator directory_entry() const{
			return directory_entry{filePath};
		}

		explicit operator path() const{
			return path();
		}

		bool operator==(const File &rhs) const {
			return filePath == rhs.filePath;
		}

		bool operator!=(const File &rhs) const {
			return !(rhs == *this);
		}

		[[nodiscard]] path absolutePath() const{
			return absolute(this->path());
		}

		[[nodiscard]] std::string extension() const {
			return filePath.extension().string();
		}

		[[nodiscard]] std::string stem() const {
			return filePath.stem().string();
		}

		[[nodiscard]] std::string filename() const {
			return filePath.filename().string();
		}

		[[nodiscard]] std::string filename_full() const {
			return (isDir() ? "<Dir>" : "") + filePath.filename().string();
		}

		[[nodiscard]] path path() const {
			return filePath;
		}

		[[nodiscard]] bool exist() const {
			return std::filesystem::exists(filePath);
		}

		[[maybe_unused]] bool deleteFile() const {  // NOLINT(*-use-nodiscard)
			return exist() && (isDir() ? remove(path()) : std::filesystem::remove_all(path()));
		}

		[[maybe_unused]] void deleteFileQuiet() const {  // NOLINT(*-use-nodiscard)
			isDir() ? remove(path()) : std::filesystem::remove_all(path());
		}

		[[nodiscard]] bool copy(const std::filesystem::path& dest) const {
			try {
				std::filesystem::copy(path(), dest);
				return true;
			}
			catch ([[maybe_unused]] std::error_code& ignore) {
				return false;
			}
		}

		[[nodiscard]] bool copy(const File& dest) const {
			return copy(dest.path());
		}

		[[nodiscard]] bool isDir() const {
			return is_directory(path());
		}

		[[nodiscard]] bool isRegular() const {
			return is_regular_file(path());
		}

		[[nodiscard]] bool isHidden() const {
			return stem().starts_with('.');
		}

		[[nodiscard]] bool emptyExtension() const {
			return extension().empty();
		}

		[[maybe_unused]] bool createDir(const bool autoCreateParents = true) const { // NOLINT(*-use-nodiscard)
			return  autoCreateParents ? create_directories(path()) : create_directory(path());
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

			std::ofstream ofStream(path());
			const bool valid = ofStream.is_open();
			ofStream.close();

			return valid;
		}

		void createFileQuiet(const bool autoCreateParents = false) const {
			// ReSharper disable once CppExpressionWithoutSideEffects
			createFile(autoCreateParents);
		}

		[[nodiscard]] File getParent() const {
			return File{ path().parent_path() };
		}

		[[nodiscard]] File subFile(const std::string_view& name) const {
			if(!isDir())throw ext::RuntimeException{};

			return File{ absolutePath().append(name) };
		}

		[[nodiscard]] File subFile(const std::string& name) const {
			if(!isDir())throw ext::RuntimeException{};

			return File{ absolutePath().append(name) };
		}

		[[nodiscard]] File subFile(const char* name) const {
			if(!isDir())throw ext::RuntimeException{};

			return File{ absolutePath().append(name) };
		}

		[[nodiscard]] File find(const std::string& name) const{
			for (const auto & item : directory_iterator(path()))
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
			if (std::ofstream ofStream(path()); ofStream.is_open()) {
				ofStream << data;

				if(autoFlush)ofStream.flush();
			}
		}

		[[nodiscard]] std::vector<File> subs(const bool careDirs = false) const{
			std::vector<File> files;
			for (const auto& item : directory_iterator(path())){
				if(item.is_directory()) {
					if(careDirs)files.emplace_back(item);
				}else {
					files.emplace_back(item);
				}
			}

			return files;
		}


		template<Concepts::Invokable<void(File&&)> T>
		void forSubs(const T& consumer) const{
			for (const auto& item : directory_iterator(path())){
				consumer(File{item});
			}
		}

		template<Concepts::Invokable<void(File&&)> T>
		void forAllSubs(const T& consumer, const bool careDirs = false) const{
			for (const auto& item : directory_iterator(path())){
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
			for (const auto& item : directory_iterator(path())){
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

		template<Concepts::Invokable<void(std::string&)> T>
		[[nodiscard]] std::string readString(const T& consumer = nullptr) const {
			std::ifstream file_stream(path());

			if(!file_stream.is_open())return "";

			std::stringstream file_contents;
			std::string line;

			while (std::getline(file_stream, line)) {
				consumer(line);
				file_contents << line << '\n';
			}

			return file_contents.str();
		}

		template<Concepts::Invokable<void(const std::ofstream&)> Func>
		void writeByte(const Func& func) {
			if(std::ofstream ofStream(absolutePath(), std::ios::binary); ofStream.is_open()) {
				func(ofStream);
			}
		}

		[[nodiscard]] std::string readString() const {
			std::ifstream file_stream(path());

			if(!file_stream.is_open())return "";

			std::stringstream file_contents;
			std::string line;

			while (std::getline(file_stream, line)) {
				file_contents << line << '\n';
			}

			return file_contents.str();
		}

		friend std::ostream &operator<<(std::ostream &os, const File &file) {
			os << file.filename_full();
			return os;
		}

		using sortPred = std::pair<std::string, std::function<bool(const File&)>>;

		[[nodiscard]] std::unordered_map<std::string, std::vector<File>> sortSubs(const bool careDirs = false) const{
			std::unordered_map<std::string, std::vector<File>> map;
			forAllSubs([&map](File&& file){
				const std::string& extension = file.extension();
				map[extension.empty() ? static_cast<std::string>(EMPTY_EXTENSION) : extension].push_back(file);
			}, careDirs);

			return map;
		}

		[[nodiscard]] std::unordered_map<std::string, std::vector<File>> sortSubsBy(const std::span<sortPred>& standards, const bool careDirs = false) const{
			std::unordered_map<std::string, std::vector<File>> map;

			forAllSubs([&standards, &map](File&& file){
				if (const auto it = std::ranges::find_if(standards, [&file](const sortPred& pair){
					return pair.second(file);
				}); it != standards.end()) {
					map[it->first].push_back(file);
				}
			}, careDirs);

			return map;
		}

		static std::unordered_map<std::string, std::vector<File>> sortBy(const std::span<File>& files, const std::span<sortPred>& standards){
			std::unordered_map<std::string, std::vector<File>> map;

			for (const File& file : files) {
				if (
					auto it = std::ranges::find_if(standards, [&file](const sortPred& pair){
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