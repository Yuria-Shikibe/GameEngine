//
// Created by Matrix on 2023/11/30.
//

module;

export module GlyphArrangement;

import Container.Pool;
import Font;

import Graphic.Color;
import Container.Pool;
import Geom.Shape.Rect_Orthogonal;
import GL.Texture.TextureRegionRect;
import Geom.Vector2D;

import Align;

import <vector>;
import <string>;
import <algorithm>;
import <functional>;
import <memory>;
import <memory_resource>;
import <unordered_map>;

export namespace Font {
	constexpr int MAX_CAHCE = 1000;
	typedef signed long FT_ULong;

	std::unordered_map<std::string_view, const FontFlags*> parserableFonts{};
	std::unordered_map<std::string_view, Graphic::Color> parserableColors{};

	void registerParserableFont(const Font::FontFlags* const flag) {
		parserableFonts.insert_or_assign(flag->fullname(), flag);
	}

	void registerParserableFont(const std::string_view name, const Font::FontFlags* const flag) {
		parserableFonts.insert_or_assign(name, flag);
	}

	//TODO color managements
	void registerParserableColor(const std::string_view name, const Graphic::Color& color) {
		parserableColors.insert_or_assign(name, color);
	}

	float normalize(const FT_ULong pos) {
		return static_cast<float>(pos) / 64.0f;
	}

	struct GlyphVertData {
		const GL::TextureRegionRect* region{nullptr};
		Graphic::Color fontColor{};

		float u0{0}, v0{0};
		float u1{0}, v1{0};

		void move(const float x, const float y) {
			u0 += x;
			u1 += x;

			v0 += y;
			v1 += y;
		}

		void moveX(const float x) {
			u0 += x;
			u1 += x;
		}

		void moveY(const float y) {
			v0 += y;
			v1 += y;
		}
	};

	struct GlyphLayout { // NOLINT(*-pro-type-member-init)
		Geom::Vector2D offset{};
		//TODO uses pools!
		std::vector<GlyphVertData> toRender{};

		float maxWidth{std::numeric_limits<float>::max()};

		Geom::Shape::OrthoRectFloat bound{};

		std::string last{};

		size_t count = 0;

		void reset() {
			last.clear();
			maxWidth = std::numeric_limits<float>::max();
			bound.set(0, 0, 0, 0);
			offset.setZero();
		}

		void clear() {
			bound.setSize(0, 0);
			bound.setSrc(0, 0);
			count = 0;
		}

		void move(const float x, const float y) {
			offset.add(x, y);
		}

		void move(const Geom::Vector2D& vec) {
			move(vec.x, vec.y);
		}

		void setAlign(const Align::Mode align) {
			if(align & Align::Mode::top) {
				bound.setSrcY(-bound.getHeight());
			}else if(align & Align::Mode::bottom){
				bound.setSrcY(0.0f);
			}else { //centerY
				bound.setSrcY(-bound.getHeight() * 0.5f);
			}

			if(align & Align::Mode::right) {
				bound.setSrcX(-bound.getWidth());
			}else if(align & Align::Mode::left){
				bound.setSrcX(0.0f);
			}else { //centerX
				bound.setSrcX(-bound.getWidth() * 0.5f);
			}
		}

		void render() const;

		void render(float progress) const;

		[[nodiscard]] GlyphLayout() = default;
	};

	//TODO cache support. maybe?
	// struct GlyphLayoutCache {
	// 	size_t frequency{0};
	//
	// 	std::shared_ptr<GlyphLayout> data{nullptr};
	// };
	//
	// std::unordered_map<std::string, GlyphLayoutCache> layoutCache(MAX_CAHCE);

	Containers::Pool<GlyphLayout> layoutPool{250, [](GlyphLayout* item) {
		item->reset();
	}};

	std::shared_ptr<GlyphLayout> obtainLayoutPtr() {
		return layoutPool.obtainShared();
	}

	struct ModifierableData;

	struct TypesettingTable {
		float spaceSpaceing{-1};
		float lineSpacing{-1};
		float currentScale{1.0f};
		float paragraphSpacing{-1};

		float additionalYOffset{0.0f}; // This happens when the font changes, maybe

		Geom::Vector2D offset{};

		std::vector<GlyphVertData*> currentLineData{};
		std::vector<std::function<void(const ModifierableData&)>> endlineOperation{};
		Geom::Shape::OrthoRectFloat currentLineBound{};

		//TODO uses stack for multi fallback?
		Graphic::Color currentColor = Graphic::Colors::WHITE;
		Graphic::Color fallbackColor = Graphic::Colors::WHITE;

		const FontFlags* defaultFont{nullptr};
		const FontFlags* currentFont{nullptr};
		const FontFlags* fallbackFont{nullptr};

		[[nodiscard]] explicit TypesettingTable(const FontFlags* font);

		[[nodiscard]] TypesettingTable() = default;

		void set(const FontFlags* const currentFont) {
			this->currentFont = currentFont;
			spaceSpaceing = currentFont->data->spaceSpacing;
			lineSpacing = currentFont->data->lineSpacingMin * 1.8f;
			paragraphSpacing = lineSpacing * 1.1f;
		}

		[[nodiscard]] float padTop_line() const {
			return currentScale * lineSpacing;
		}

		[[nodiscard]] float padTop_paragraph() const {
			return currentScale * paragraphSpacing;
		}

		void reset() {
			currentColor = fallbackColor = Graphic::Colors::WHITE;
			currentScale = 1.0f;
			set(defaultFont);
			fallbackFont = defaultFont;

			offset.setZero();
			additionalYOffset = 0;
		}
	};

	struct ModifierableData {
		TypesettingTable& context;
		Geom::Vector2D& cursorPos;
		const Font::CharData*& charData;
		GlyphLayout& layout;

		[[nodiscard]] ModifierableData(TypesettingTable& context, Geom::Vector2D& vec2,
			const Font::CharData*& data, GlyphLayout& layout)
			: context(context),
			cursorPos(vec2),
			charData(data),
			layout(layout) {
		}
	};

namespace ParserFunctions {
	void setScl(const ModifierableData& data, float target);

	void resetScl(const ModifierableData& data) {
		setScl(data, 1.0f);
	}

	void endLine(const ModifierableData& data);
}

	class TokenParser {
	public:
		virtual ~TokenParser() = default;

		std::unordered_map<std::string_view, std::function<void(const std::string_view, const ModifierableData& data)>> modifier{};

		virtual void parse(std::string_view token, const ModifierableData& data) const;

		void operator()(const std::string_view token, const ModifierableData& data) const {
			parse(token, data);
		}
	};

	class CharParser {
	public:
		virtual ~CharParser() = default;

		std::unordered_map<char, std::function<void(const ModifierableData& data)>> modifier{};

		virtual void parse(const char& token, const ModifierableData& data) const {
			modifier.at(token)(data);
		}

		[[nodiscard]] bool contains(const char c) const {
			return modifier.contains(c);
		}

		void operator()(const char c, const ModifierableData& data) const {
			parse(c, data);
		}
	};

	class GlyphParser {
	public:
		virtual ~GlyphParser() = default;

		const char TokenSignal = '$'; //$$for$
		const char TokenBeginCode = '{';
		const char TokenEndCode = '}';

		Font::FontCache* fontLib{nullptr};

		mutable TypesettingTable context{};

		std::unique_ptr<TokenParser> tokenParser{std::make_unique<TokenParser>()};
		std::unique_ptr<CharParser> charParser{std::make_unique<CharParser>()};

		/**
		 * \brief Customizable Fields
		 */
		[[nodiscard]] explicit GlyphParser(const FontFlags* const defFont) : context{defFont}{

		}

		virtual void parse(std::shared_ptr<GlyphLayout> layout, std::string_view text, float newMaxWidth = std::numeric_limits<float>::max()) const;

		[[nodiscard]] std::shared_ptr<GlyphLayout> parse(const std::string_view text) const {
			const auto layout = std::make_shared<GlyphLayout>();

			parse(layout, text);

			return layout;
		}

		std::shared_ptr<GlyphLayout> operator()(const std::string_view text) const {
			//TODO complete glyph layout cache;
			/*if(
				const auto itr = layoutCache.find(text);
				itr != layoutCache.end()
			) {
				itr->second.frequency++;
				return itr->second.data;
			}*/

			return parse(text);
		}

		[[nodiscard]] GlyphParser() : GlyphParser(nullptr){

		}
	};

	GlyphParser* glyphParser = nullptr;

	void initParser(const FontFlags* defFont);
}
