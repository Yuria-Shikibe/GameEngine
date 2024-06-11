//
// Created by Matrix on 2024/6/8.
//

export module Assets.TexturePage;

import std;
import GL.Texture.Texture2D;
import GL.Texture.TextureRegion;
import ext.Heterogeneous;

import Graphic.Pixmap;
import Geom.Vector2D;
import Geom.Rect_Orthogonal;
import GL.Texture.Texture2D;
import GL.Texture.TextureNineRegion;

import Image;
import Image.Svg;

import OS.File;

export namespace Assets{
	struct BitmapLoadData{
		virtual ~BitmapLoadData() = default;

		std::string stemName{};
		Graphic::Pixmap bitmapData{};


		std::function<bool(BitmapLoadData&)> modifier{};

		[[nodiscard]] BitmapLoadData() = default;

		[[nodiscard]] explicit BitmapLoadData(std::string_view familyName)
			: stemName{familyName}{}

		[[nodiscard]] BitmapLoadData(const std::string_view familyName, const Graphic::Pixmap& bitmapData, std::function<bool(BitmapLoadData&)>&& modifier = nullptr)
			: stemName{familyName},
			  bitmapData{bitmapData}, modifier{std::move(modifier)}{}

		[[nodiscard]] BitmapLoadData(const std::string_view familyName, Graphic::Pixmap&& bitmapData, std::function<bool(BitmapLoadData&)>&& modifier = nullptr)
			: stemName{familyName},
			  bitmapData{std::move(bitmapData)}, modifier{std::move(modifier)}{}

		virtual void load(){
			if(modifier)modifier(*this);
		}

		[[nodiscard]] constexpr bool valid() const noexcept{
			return bitmapData.valid();
		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept{
			return valid();
		}
	};

	struct FileImportData : BitmapLoadData{
		OS::File srcFile{};

		using BitmapLoadData::BitmapLoadData;

		[[nodiscard]] FileImportData(const std::string_view familyName, const OS::File& srcFile)
			: BitmapLoadData{familyName},
			  srcFile{srcFile}{}

		[[nodiscard]] explicit FileImportData(const OS::File& srcFile)
	: BitmapLoadData{srcFile.filename()},
	  srcFile{srcFile}{}

		void load() override{
			//TODO more branch handle
			if(const auto extension = srcFile.extension(); extension == ".svg"){
				bitmapData = ext::svgToBitmap(srcFile);
			}else{
				bitmapData.loadFrom(srcFile);
			}

			BitmapLoadData::load();
		}
	};

	struct SvgFileImportData : FileImportData{
		Geom::Point2U size{};
		bool mulWhite{};

		using FileImportData::FileImportData;

		[[nodiscard]] SvgFileImportData(const std::string_view familyName, const OS::File& srcFile, const Geom::Point2U size = {},
			const bool mulWhite = false)
			: FileImportData{familyName, srcFile},
			  size{size},
			  mulWhite{mulWhite}{}

		[[nodiscard]] SvgFileImportData(const OS::File& srcFile, const Geom::Point2U size = {},
			const bool mulWhite = false)
			: FileImportData{srcFile},
			  size{size},
			  mulWhite{mulWhite}{}

		void load() override{
			bitmapData = ext::svgToBitmap(srcFile, size.x, size.y);
			if(mulWhite)bitmapData.mulWhite();

			BitmapLoadData::load();
		}
	};

	class TexturePage{
		std::string name{};
		int margin{0};

		std::vector<std::unique_ptr<GL::Texture2D>> textures{};

		//TODO uses unique ptr to make the page moveable?
		ext::StringHashMap<GL::TextureRegion> textureRegions{};
		unsigned version{};

		ext::StringHashMap<std::unique_ptr<BitmapLoadData>> requests{};

	public:
		TexturePage* dependency{};

		[[nodiscard]] auto& getRequests() noexcept{ return requests; }

		[[nodiscard]] TexturePage() = default;

		[[nodiscard]] explicit TexturePage(const std::convertible_to<std::string_view> auto name)
			: name{name}{}

		[[nodiscard]] explicit TexturePage(std::string&& name)
			: name{std::move(name)}{}

		[[nodiscard]] std::string_view getName() const noexcept{ return name; }

		[[nodiscard]] int getMargin() const{ return margin; }

		void setMargin(const int margin){ this->margin = margin; }

		template <Concepts::Derived<BitmapLoadData> T, typename ...Args>
		GL::TextureRegion* pushRequest(std::string&& name, Args&& ...args){
			requests.try_emplace(name, std::make_unique<T>(name, std::forward<Args>(args) ...));
			auto [rst, success] = textureRegions.try_emplace(std::move(name), GL::TextureRegion{});

			if(success){
				return &rst->second;
			}

			return nullptr;
		}


		GL::TextureRegion* pushRequest(const std::string_view name, std::unique_ptr<BitmapLoadData>&& task){
			requests.try_emplace(std::string(name), std::move(task));
			auto [rst, success] = textureRegions.try_emplace(std::string(name), GL::TextureRegion{});

			if(success){
				return &rst->second;
			}

			return nullptr;
		}

		template <Concepts::Derived<BitmapLoadData> T, typename ...Args>
		GL::TextureRegion* pushRequest(std::string_view name, Args&& ...args){
			requests.try_emplace(std::string(name), std::make_unique<T>(name, std::forward<Args>(args) ...));
			auto [rst, success] = textureRegions.try_emplace(std::string(name), GL::TextureRegion{});

			if(success){
				return &rst->second;
			}

			return nullptr;
		}

		template <Concepts::Derived<FileImportData> T = Assets::FileImportData, typename ...Args>
		GL::TextureRegion* pushRequest(const OS::File& file, Args&& ...args){
			auto name = file.stem();
			requests.try_emplace(name, std::make_unique<T>(file, std::forward<Args>(args) ...));
			auto [rst, success] = textureRegions.try_emplace(std::move(name), GL::TextureRegion{});

			if(success){
				return &rst->second;
			}

			return nullptr;
		}

		auto& getRegions() noexcept {
			return textureRegions;
		}

		[[nodiscard]] constexpr std::size_t getSize() const noexcept{
			return textures.size();
		}

		[[nodiscard]] auto& getTextures() const noexcept {return textures;}
		[[nodiscard]] auto& getTextures() noexcept {return textures;}

		[[nodiscard]] constexpr bool hasDependency() const noexcept{return dependency != nullptr;}

		[[nodiscard]] auto getVersion() const noexcept{ return version; }

		[[nodiscard]]
		GL::TextureRegion* find(const std::string_view regionName, GL::TextureRegion* fallBack = nullptr) {
			if(const auto itr = textureRegions.find(regionName); itr != textureRegions.end()) {
				return &itr->second;
			}

			return fallBack;
		}

		TexturePage(const TexturePage& other) = delete;

		TexturePage& operator=(const TexturePage& other) = delete;

		TexturePage(TexturePage&& other) noexcept = default;

		TexturePage& operator=(TexturePage&& other) noexcept = default;
	};
}
