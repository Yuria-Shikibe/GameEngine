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
	namespace Load{
		class TexturePacker;
		struct PageDataLoader;
	}

	struct BitmapLoadData{
		virtual ~BitmapLoadData() = default;

		std::string stemName{};

		//TODO is this name necessary?
		std::string name{};
		Graphic::Pixmap bitmapData{};


		std::function<bool(BitmapLoadData&)> modifier{};

		[[nodiscard]] BitmapLoadData() = default;

		[[nodiscard]] BitmapLoadData(std::string_view familyName, std::string_view name)
			: stemName{familyName},
			  name{name}{}

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

		[[nodiscard]] FileImportData(std::string_view familyName, const OS::File& srcFile)
			: BitmapLoadData{familyName, srcFile.filename()},
			  srcFile{srcFile}{}

		[[nodiscard]] explicit FileImportData(const OS::File& srcFile)
	: BitmapLoadData{srcFile.filename(), srcFile.filename()},
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

		[[nodiscard]] SvgFileImportData(std::string_view familyName, const OS::File& srcFile, const Geom::Point2U& size,
			const bool mulWhite = false)
			: FileImportData{familyName, srcFile},
			  size{size},
			  mulWhite{mulWhite}{}

		[[nodiscard]] SvgFileImportData(const OS::File& srcFile, const Geom::Point2U& size,
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

	struct TexturePage{
		std::string name{};
		std::vector<std::unique_ptr<GL::Texture2D>> textures{};
		ext::StringMap<GL::TextureRegion> textureRegions{};
		unsigned version{};

		TexturePage* dependency;

		ext::StringHashMap<std::unique_ptr<BitmapLoadData>> toLoad{};

		friend Load::TexturePacker;
		friend Load::PageDataLoader;
	public:

		template <Concepts::Derived<BitmapLoadData> T, typename ...Args>
		GL::TextureRegion* pullRequest(std::string_view name, Args&& ...args){
			toLoad.try_emplace(std::string(name), std::make_unique<T>(name, std::forward<Args>(args) ...));
			auto [rst, success] = textureRegions.try_emplace(std::string(name), GL::TextureRegion{});

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

		[[nodiscard]] constexpr bool hasDependency() const noexcept{return dependency != nullptr;}
	};
}
