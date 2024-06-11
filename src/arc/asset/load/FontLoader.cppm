module;

#include <ft2build.h>
#include <freetype/freetype.h>

export module Assets.Load.FontLoader;

export import Font;
export import Font.Manager;
import Assets.Load.Core;
import Assets.TexturePage;

import OS.File;
import ext.RuntimeException;
import ext.Heterogeneous;
import Core.IO.Specialized;

import Assets.Load.TexturePacker;
import std;

export namespace Assets::Load{
	class FontLoader : public LoadTask{
	protected:
		bool quickInit = false;
		std::vector<std::unique_ptr<Font::FontFace>> flags{};
		Assets::TexturePage* bindedPage{nullptr};

		static void exitLoad(std::string_view fontName) {
			throw ext::RuntimeException{std::format("Font load failed during {}.", fontName)};
		}

		static void processCustomeData(Font::FontFace& currentFace, std::unordered_map<Font::CharCode, Font::FontData_Preload>& fontDatas){
			for(auto& customeData : currentFace.customeCharDatas){
				if(!customeData.forceOverride && fontDatas.contains(customeData.code))continue;

				if(customeData.hasCopyTarget()){
					if(auto itr = fontDatas.find(customeData.copyTarget); itr != fontDatas.end()){
						customeData.glyphMatrices = itr->second.charData.glyphMatrices;
					}
				}

				if(customeData.dataModifier){
					customeData.dataModifier(customeData);
				}

				fontDatas.insert_or_assign(customeData.code, Font::FontData_Preload{customeData.code, customeData.glyphMatrices, std::move(customeData.data)});
			}
		}

		void loadFont(Font::FontFace& currentFace) const{
			const std::string fontFullName = currentFace.fullname();

			if(!currentFace.face)exitLoad(fontFullName);

			std::unordered_map<Font::CharCode, Font::FontData_Preload> fontDatas{};

			// Graphic::Pixmap maxMap{};
			FT_Set_Pixel_Sizes(currentFace.face, 0, currentFace.size);

			for (const Font::CharCode i : currentFace.segments){
				if(const FT_Face face = currentFace.tryLoad(i)){ // NOLINT(*-misplaced-const)
					if(!face->glyph) {
						exitLoad(fontFullName);
					}

					//TODO glyph effects
					//TODO should this thing be here???
					if (currentFace.loader && face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
						if (currentFace.loader(face)) {
							exitLoad(fontFullName);
						}
					}

					fontDatas.insert_or_assign(i, Font::FontData_Preload{i, face->glyph});
				}
			}

			processCustomeData(currentFace, fontDatas);

			std::ostringstream ss{};

			for (auto& fontData : fontDatas | std::views::values){
				ss.str("");
				ss << fontFullName << std::to_string(fontData.charCode);
				fontData.charData.region = bindedPage->pushRequest(ss.view(), std::make_unique<BitmapLoadData>(
					ss.view(), std::move(fontData.pixmap), [](const Assets::BitmapLoadData& data){data.bitmapData.flipY(); return true;}));
			}

			currentFace.data = std::make_unique<Font::FontData>(fontDatas);

			if(currentFace.data->lineSpacingDef < 0) {
				currentFace.data->lineSpacingDef = Font::normalizeLen(currentFace.face->size->metrics.ascender);
			}
		}

		void loadAllFace() const {
			for(const auto& params: flags){
				if(!params->face) {
					if(FT_New_Face(Font::GlobalFreeTypeLib, params->fontFile.absolutePath().string().data(), 0, &params->face)) {
						exitLoad(params->fullname());
					}
					//Correct the style name if necessary
					params->familyName = params->face->family_name;
					params->styleName = params->face->style_name;
				}
			}
		}

		void assignID() const {
			std::vector<std::string_view> loadedFamily;
			for(const auto& [index, face] : flags | std::ranges::views::enumerate) {
				// ReSharper disable once CppUseStructuredBinding

				const auto itr = std::ranges::find(loadedFamily, face->familyName);
				const auto dst = std::ranges::distance(loadedFamily.cbegin(), itr);

				if(itr == loadedFamily.end()){
					loadedFamily.push_back(face->familyName);
				}

				face->fullID |= static_cast<unsigned char>(index) << Font::FontFace::fontOffset;
				face->fullID |= static_cast<unsigned char>(dst) << Font::FontFace::familyOffset;
				face->fullID |= Font::getStyleID(face->styleName) << Font::FontFace::styleOffset;
			}
		}

		bool load() { //Dose not support hot load!
			if(!Font::GlobalFreeTypeLib){
				throw ext::RuntimeException{"Missing FT Lib"};
			}

			if(!bindedPage){
				throw ext::NullPointerException{"Missing Binded Page"};
			}

			loadAllFace();

			//Cache prepare
			for(const auto& params : flags) {
				loadFont(*params);
			}

			assignID();

			done();
			handler.join();

			return finished;
		}

	public:
		[[nodiscard]] FontLoader() = default;

		FontLoader(const FontLoader& other) = delete;

		FontLoader(FontLoader&& other) noexcept = delete;

		FontLoader& operator=(const FontLoader& other) = delete;

		FontLoader& operator=(FontLoader&& other) noexcept = delete;

		[[nodiscard]] constexpr Assets::TexturePage* getBindedPage() const noexcept{ return bindedPage; }

		constexpr void setBindedPage(Assets::TexturePage* const bindedPage) noexcept{ this->bindedPage = bindedPage; }

		Font::FontFace* registerFont(Font::FontFace* fontFlags) {
			flags.emplace_back(fontFlags);
			return fontFlags;
		}

		std::future<bool> launch(const std::launch policy) override{
			return std::async(policy, &FontLoader::load, this);
		}

		Font::FontManager cropStorage() && {
			return Font::FontManager(std::move(flags));
		}
	};

	class QuickInitFontLoader : protected FontLoader{
	public:
		[[nodiscard]] QuickInitFontLoader() = default;

		using FontLoader::registerFont;
		using FontLoader::cropStorage;
		using FontLoader::setBindedPage;
		// ReSharper disable once CppHidingFunction
		void blockLoad(Assets::TexturePage* const bindedPage, const OS::File& cacheDir){
			setBindedPage(bindedPage);

			blockLoad(cacheDir);
		}

		void blockLoad(const OS::File& cacheDir){
			LoadManager instantManager{"Instant Font Temp Loader"};
			TexturePacker instantPacker{};

			instantPacker.setCacheDir(cacheDir);
			instantPacker.pushPage(bindedPage);
			instantManager.registerTask(instantPacker);
			instantManager.registerTask(*this);
			instantManager.launch();
			while(!instantManager.isFinished()){
				instantManager.processRequests();
			}
		}
	};
}
