//
// Created by Matrix on 2024/1/1.
//

export module Game.ContentLoader;

export import Game.Content.ContentType;

import std;
import Concepts;
import ext.RuntimeException;

import ext.Heterogeneous;

export namespace Game{
	class ContentLoader {
	protected:
		std::unordered_map<std::string_view, std::unique_ptr<ContentTrait>> contents{};

	public:
		ContentTrait* find(const std::string_view name){
			const auto itr = this->contents.find(name);

			if(itr == this->contents.end())return nullptr;
			return itr->second.get();
		}

		template <Concepts::Derived<ContentTrait> T>
			requires requires{
				new T{std::declval<std::string_view>()};
			}
		T* registerContent(const std::string_view name, Concepts::InvokeNullable<void(T*)> auto&& initilazer = nullptr){
			auto rst = contents.try_emplace(name, std::make_unique<T>(name));

			if(rst.second){
				T* t = static_cast<T*>(rst.first->second.get());
				if constexpr (!std::same_as<decltype(initilazer), std::nullptr_t>){
					initilazer(t);
				}

				return t;
			}

			throw ext::IllegalArguments{std::format("Content Name Duplicated: {}", name)};
		}

		void loadTexture(const OS::FileTree& fileTree, Graphic::TextureAtlas& atlas){
			for (const auto& content : contents | std::ranges::views::values){
				content->pullLoadRequest(atlas, fileTree, "");
			}
		}
	};
}








