//
// Created by Matrix on 2023/11/22.
//

module ;

export module Assets.Bundle;

import ext.Json;
import std;

export namespace Assets{
	class Bundle;
	struct BundleLoadable{
		virtual ~BundleLoadable() = default;

		virtual void loadBundle(const Bundle* bundle) = 0;
	};

	class Bundle{
		std::vector<BundleLoadable*> bundleRequesters{};

		ext::json::JsonValue currentBundle{};

	public:
		using KeyType = std::string_view;
		auto& getBundles() {
			return currentBundle.asObject();
		}

		const auto& getBundles() const {
			return currentBundle.asObject();
		}

		[[nodiscard]] std::string_view find(const KeyType key, const std::string_view def = "Not_Found") const{
			const auto& map = getBundles();
			if(const auto itr = map.find(key); itr != map.end()){
				if(const auto data = itr->second.tryGetValue<std::string>()){
					return *data;
				}
			}
			return def;
		}

		template <typename ...T>
		[[nodiscard]] std::string format(const KeyType key, T&& args) const{
			return std::vformat(find(key), std::make_format_args(std::forward<T>(args) ...));
		}

		[[nodiscard]] std::string_view operator[](const KeyType key) const{
			return find(key);
		}
	};
}
