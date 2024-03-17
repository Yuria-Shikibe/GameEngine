//
// Created by Matrix on 2024/1/1.
//

export module Game.ContentLoader;

export import Game.ContentType;

import std;
import Concepts;
import RuntimeException;

export namespace Game{
	class ContentLoader {
	protected:
		std::unordered_map<std::string_view, std::unique_ptr<Content>> contents{};

	public:
		Content* find(const std::string_view name){
			const auto itr = this->contents.find(name);

			if(itr == this->contents.end())return nullptr;
			return itr->second.get();
		}

		template <Concepts::Derived<Content> T>
		T* registerContent(const std::string_view name, Concepts::Invokable<void(T*)> auto&& initilazer){
			auto rst = contents.try_emplace(name, std::make_unique<T>());

			if(rst.second){
				T* t = rst.first->second;
				initilazer(t);
				return t;
			}

			throw ext::IllegalArguments{std::format("Content Name Duplicated: {}", name)};
		}

		void load(Graphic::TextureAtlas* atlas){
			for (const auto& content : contents | std::ranges::views::values){
				content->pullRequest(atlas);
			}
		}
	};
}








