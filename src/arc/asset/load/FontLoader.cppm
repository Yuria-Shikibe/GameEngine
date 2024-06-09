module;

#include <ft2build.h>
#include <freetype/freetype.h>

export module Assets.Load.FontLoader;

export import Font;
import Assets.Load.Core;
import Assets.TexturePage;

import OS.File;
import ext.RuntimeException;
import ext.Heterogeneous;

import Assets.Load.TexturePacker;

export namespace Assets::Load{
	class FontLoader : public LoadTask{
	protected:
		bool quickInit = false;
		std::vector<std::unique_ptr<Font::FontFace>> flags{};
		// std::unique_ptr<Font::FontStorage> atlas{nullptr};
		Assets::TexturePage* bindedPage{nullptr};

		static void exitLoad(std::string_view fontName) {
			throw ext::RuntimeException{std::format("Font load failed during {}.", fontName)};
		}

		void processCustomeData(Font::FontFace& currentFace, Font::PreloadDataSet& fontDatas){
			for(auto& customeData : currentFace.customeCharDatas){
				if(!customeData.forceOverride && fontDatas.contains(customeData.code))continue;

				if(customeData.hasCopyTarget()){
					if(auto itr = fontDatas.find(customeData.copyTarget); itr != fontDatas.end()){
						customeData.glyphMatrices = itr->glyphMatrices;
					}
				}

				if(customeData.dataModifier){
					customeData.dataModifier(customeData);
				}

				fontDatas.emplace(customeData.glyphMatrices, customeData.code, std::move(customeData.data));
			}
		}

		void loadFont(Font::FontFace& currentFace) const{
			const std::string fontFullName = currentFace.fullname();

			if(!currentFace.face)exitLoad(fontFullName);

			Font::PreloadDataSet fontDatas{};

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

					fontDatas.emplace(i, face->glyph);
				}
			}

			processCustomeData(currentFace, fontDatas);

			for (auto& fontData : fontDatas){
				fontData.charData.region = bindedPage->pullRequest<BitmapLoadData>(fontFullName + std::to_string(fontData.charCode), std::move(fontData.pixmap), [](const Assets::BitmapLoadData& data){data.bitmapData.flipY();});
			}

			currentFace.data = std::make_unique<Font::FontData>(fontDatas);
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

				face->internalID |= static_cast<unsigned char>(index) << Font::FontFace::fontOffset;
				face->internalID |= static_cast<unsigned char>(dst) << Font::FontFace::familyOffset;
				face->internalID |= Font::getStyleID(face->styleName) << Font::FontFace::styleOffset;
			}
		}

		//This virtual is not necessary, just fuck the IDE's warnings
		virtual void load() { //Dose not support hot load!
			Font::loadLib();

			loadAllFace();

			//Cache prepare
			for(const auto& params : flags) {
				loadFont(*params);
			}

			assignID();

			handler.join();
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

		std::future<void> launch(std::launch policy) override{
			return std::async(policy, &FontLoader::load, this);
		}
	};

	class QuickInitFontLoader : public FontLoader{
	public:
		[[nodiscard]] QuickInitFontLoader() = default;

		// ReSharper disable once CppHidingFunction
		void load() override{
			LoadManager instantManager{};
			TexturePacker instantPacker{};
			instantPacker.pushPage(bindedPage);
			instantManager.registerTask(instantPacker);
			instantManager.registerTask(*this);
			instantManager.launch();
			while(!instantManager.isFinished()){
				instantManager.processRequests();
			}

			instantManager.requestDone();
		}
	};
}
