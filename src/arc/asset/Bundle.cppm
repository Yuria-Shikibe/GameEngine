module ;

export module Assets.Bundle;

import ext.Json;
import std;
import ext.Heterogeneous;
import OS.File;

export namespace Assets{
	class Bundle;
	struct BundleLoadable{
		virtual ~BundleLoadable() = default;

		virtual void loadBundle(const Bundle* bundle) = 0;
	};

	class Bundle{
	public:
		static constexpr std::string_view Loacle = "loacle";
		static constexpr std::string_view NotFound = "???Not_Found???";
	private:
		std::vector<BundleLoadable*> bundleRequesters{};

		ext::json::JsonValue currentBundle{};
		ext::json::JsonValue fallbackBundle{};
		std::locale currentLocal{};

		static std::vector<std::string_view> getKeyCategory(const std::string_view key){
			std::vector<std::string_view> keyCategories{};

			int lastDotPos = 0;
			for(auto [i, c] : key | std::ranges::views::enumerate){
				if(c == '.'){
					keyCategories.push_back(key.substr(lastDotPos, i - lastDotPos));
					lastDotPos = i + 1;
				}
			}

			keyCategories.push_back(key.substr(lastDotPos));


			return keyCategories;
		}

		const auto& getCategory(const std::string_view category) const {
			const auto& map = getBundles();

			if(const auto itr = map.find(category); itr != map.end()){
				if(itr->second.is<ext::json::Object>())return itr->second.asObject();
			}

			return map;
		}

		static ext::json::JsonValue loadFile(const OS::File& file){
			const auto str = file.readString([](std::string& line){
				return std::erase_if(line, std::not_fn(ext::json::notIgnore));
			});

			ext::json::JsonValue jval{};
			jval.parseObject(str);
			return jval;
		}

		static std::optional<std::string_view> find(const std::vector<std::string_view>& dir, const ext::json::Object* last){
			if(dir.size() > 1){
				for(auto cates : std::ranges::subrange{dir.begin(), dir.end() - 1}){
					if(const auto itr = last->find(cates); itr != last->end()){
						if(itr->second.is<ext::json::Object>()){
							last = &itr->second.asObject();
						}else{
							break;
						}
					}
				}
			}

			if(const auto itr = last->find(dir.back()); itr != last->end()){
				if(const auto data = itr->second.tryGetValue<std::string>()){
					return *data;
				}
			}

			return std::nullopt;
		}

	public:
		[[nodiscard]] Bundle(){
			currentBundle.asObject();
		}

		[[nodiscard]] explicit Bundle(const OS::File& file){
			load(file);
		}

		template <ext::json::JsonValue Bundle::* ptr = &Bundle::currentBundle>
		ext::json::Object& getBundles() {
			return (this->*ptr).asObject();
		}

		template <ext::json::JsonValue Bundle::* ptr = &Bundle::currentBundle>
		const ext::json::Object& getBundles() const {
			return (this->*ptr).asObject();
		}

		void loadFallback(ext::json::JsonValue&& jsonValue){
			fallbackBundle = std::move(jsonValue);
			fallbackBundle.asObject();
		}

		void load(ext::json::JsonValue&& jsonValue){
			currentBundle = std::move(jsonValue);
			currentBundle.asObject();
			for (const auto bundleRequester : bundleRequesters){
				bundleRequester->loadBundle(this);
			}
		}

		void loadFallback(const OS::File& file){
			loadFallback(loadFile(file));
		}

		void load(const OS::File& file){
			load(loadFile(file));
		}

		void load(const OS::File& target, const OS::File& fallback){
			load(target);
			loadFallback(fallback);
		}

		[[nodiscard]] std::string_view find(const std::initializer_list<std::string_view> keyWithConstrains, const std::string_view def) const{
			std::vector<std::string_view> flat{};
			flat.reserve(keyWithConstrains.size());

			for (const auto key : keyWithConstrains){
				auto dir = getKeyCategory(key);
				if(dir.empty())continue;
				flat.append_range(dir);
			}

			auto* last = &currentBundle.asObject();

			auto rst = find(flat, last);

			if(!rst){
				last = &fallbackBundle.asObject();
				rst = find(flat, last);
			}

			return rst.value_or(def);
		}

		[[nodiscard]] std::string_view find(const std::initializer_list<std::string_view> keyWithConstrains) const{
			return find(keyWithConstrains, NotFound);
		}

		[[nodiscard]] std::string_view find(const std::string_view key, const std::string_view def) const{
			auto dir = getKeyCategory(key);
			auto* last = &currentBundle.asObject();

			auto rst = find(dir, last);

			if(!rst){
				last = &fallbackBundle.asObject();
				rst = find(dir, last);
			}

			return rst.value_or(def);
		}

		[[nodiscard]] std::string_view find(const std::string_view key) const{
			return find(key, key);
		}

		template <typename ...T>
		[[nodiscard]] std::string format(const std::string_view key, T&& ...args) const{
			return std::vformat(find(key), std::make_format_args(std::forward<T>(args) ...));
		}

		[[nodiscard]] std::string_view operator[](const std::string_view key) const{
			return find(key, key);
		}
	};
}
