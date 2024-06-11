//
// Created by Matrix on 2024/6/10.
//

export module Font.Manager;

import Font;
import std;

export namespace Font{
	//this will contain all the fonts with a single texture, for fast batch process
	struct FontManager {
	protected:
		std::unordered_map<CharCode, std::unordered_set<FontID>> supportedFonts{};
		std::unordered_map<FontID, std::unique_ptr<FontFace>> fonts{}; //Access it by FontFlags.internalID

	public:
		[[nodiscard]] FontManager() = default;

		[[nodiscard]] explicit FontManager(std::vector<std::unique_ptr<FontFace>>&& fontsRaw){
			fonts.reserve(fontsRaw.size());
			for(auto& value: fontsRaw) {
				//Register valid chars

				for (const auto i : value->segments){
					supportedFonts[i].insert(value->fullID);
				}

				fonts.insert_or_assign(value->fullID, std::move(value));
			}
		}

		[[nodiscard]] const FontFace* obtain(const FontFace* const flag) const {
			return obtain(flag->fullID);
		}

		[[nodiscard]] const FontFace* obtain(const FontID id) const {
			if(const auto itr = fonts.find(id); itr != fonts.end()) {
				return itr->second.get();
			}

			return nullptr;
		}

		[[nodiscard]] bool contains(const FontID id, const CharCode charCode) const {
			return supportedFonts.at(charCode).contains(id);
		}

		[[nodiscard]] const CharData* getCharData(const FontID id, const CharCode charCode) const {
			return contains(id, charCode) ? &fonts.at(id)->data->charDatas.at(charCode) : nullptr;
		}
	};
}
