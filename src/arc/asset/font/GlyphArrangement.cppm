//
// Created by Matrix on 2023/11/30.
//

module;

export module GlyphArrangement;

import Font;
import Graphic.Draw;
import Graphic.Color;
import Container.Pool;
import Geom.Shape.Rect_Orthogonal;
import Geom.Vector2D;
import GL.Texture.TextureRegionRect;
import <vector>;
import <string>;
import <algorithm>;
import <functional>;
import <memory>;
import <unordered_map>;

export namespace Font {
	constexpr int MAX_CAHCE = 1000;
	typedef signed long FT_ULong;

	std::unordered_map<std::string, unsigned int> parserableFonts{};
	std::unordered_map<std::string, Graphic::Color> parserableColors{};

	void registerParserableFont(const Font::FontFlags* const flag) {
		parserableFonts[flag->fullname()] = flag->internalID;
	}

//
//
//TODO color managements
	void registerParserableColor(const std::string& name, const Graphic::Color& color) {
		// parserableFonts.insert_or_assign(name, color);
	}

	inline float normalize(const FT_ULong pos) {
		return static_cast<float>(pos) / 64.0f;
	}

	std::unordered_map<std::string, Graphic::Color> registeredColors;

	void registerColor(const std::string& name, const Graphic::Color& color) {
		registeredColors.insert_or_assign(name, color);
	}

	struct GlyphVertData {
		const GL::TextureRegionRect* region{nullptr};
		const Graphic::Color* fontColor{};

		float u0{0}, v0{0};
		float u1{0}, v1{0};
	};

	struct GlyphLayout {
		std::shared_ptr<std::vector<GlyphVertData>> toRender{nullptr};

		void render() const {
			std::ranges::for_each(*toRender, [](const auto& glyph) {
				Graphic::Draw::vert_monochromeAll(
					glyph.region->texture(), *glyph.fontColor, Graphic::Draw::contextMixColor,
					glyph.u0, glyph.v0, glyph.region->u0, glyph.region->v0,
					glyph.u0, glyph.v1, glyph.region->u0, glyph.region->v1,
					glyph.u1, glyph.v1, glyph.region->u1, glyph.region->v1,
					glyph.u1, glyph.v0, glyph.region->u1, glyph.region->v0
				);
			});
		}
	};

	struct GlyphLayoutCache {
		size_t frequency{0};
		std::shared_ptr<std::vector<GlyphVertData>> data{nullptr};
	};

	std::unordered_map<std::string, GlyphLayoutCache> layoutCache(MAX_CAHCE);

	struct TypesettingTable {
		const FontFlags* currentFont{nullptr};
		float spaceSpaceing{-1};
		float lineSpacing{-1};
		float currentScale{1.0f};
		float paragraphSpacing{-1};

		const Graphic::Color* currentColor = &Graphic::Colors::WHITE;

		[[nodiscard]] explicit TypesettingTable(const FontFlags* const currentFont)
			: currentFont(currentFont) {
			spaceSpaceing = currentFont->data->spaceSpacing;
			lineSpacing = currentFont->data->lineSpacingMin * 1.8f;
			paragraphSpacing = lineSpacing * 1.1f;
		}

		[[nodiscard]] TypesettingTable() = default;

		void set(const FontFlags* const currentFont) {
			this->currentFont = currentFont;
			spaceSpaceing = currentFont->data->spaceSpacing;
			lineSpacing = currentFont->data->lineSpacingMin * 1.2885f;
			paragraphSpacing = lineSpacing * 1.3f;
		}

		[[nodiscard]] float padTop_line() const {
			return currentScale * lineSpacing;
		}

		[[nodiscard]] float padTop_paragraph() const {
			return currentScale * paragraphSpacing;
		}
	};

	class GlyphParser {
	public:
		virtual ~GlyphParser() = default;

		const char TokenSignal = '$'; //$$for$
		const char TokenBeginCode = '{';
		const char TokenEndCode = '}';

		mutable TypesettingTable context{};

		std::unordered_map<FT_ULong, std::function<void(Geom::Vector2D&, const Font::FontData::CharData*&)>> offseter{};
		std::unordered_map<std::string, std::function<void(std::string&, TypesettingTable&)>> tokens{};

		[[nodiscard]] explicit GlyphParser(const FontFlags* const defFont) : context{defFont}{
			offseter[' '] = [this](Geom::Vector2D& vec2, const Font::FontData::CharData* data) {
				vec2.add(this->context.spaceSpaceing, 0);
			};

			offseter['\n'] = [this](Geom::Vector2D& vec2, const Font::FontData::CharData*& data) {
				data = &Font::emptyCharData;
				vec2.add(0, -this->context.lineSpacing);
				vec2.setX(0);
			};
		}

		[[nodiscard]] virtual std::shared_ptr<std::vector<GlyphVertData>> parse(const std::string& text) const {
			constexpr auto npos = std::string::npos;

			const auto datas = std::make_shared<std::vector<GlyphVertData>>();
			datas->reserve(text.length());

			const Font::FontData::CharData* lastCharData = &Font::emptyCharData;
			bool tokenState = false;
			size_t tokenBegin = npos;
			std::string token{};
			Geom::Vector2D currentPosition{};

			for(size_t index = 0; index < text.size(); ++index) {
				const char currentChar = text.at(index);
				//Token Check
				if(currentChar == TokenSignal) {
					if(TokenSignal)tokenState = false;
					else tokenState = true;

					continue;
				}

				if(tokenState) {
					if(currentChar == TokenSignal) {
						goto process;
					}

					if(currentChar == TokenBeginCode) {
						tokenBegin = index + 1;
					}else if(currentChar == TokenEndCode) {
						if(tokenBegin != npos) {
							token = text.substr(tokenBegin, index - tokenBegin);
							tokens.at(token)(token, context);
						}

						tokenState = false;
					}

					continue;
				}

				process:

				if(
					auto itr = offseter.find(currentChar);
					itr != offseter.end()
				) {
					itr->second.operator()(currentPosition, lastCharData);
					continue;
				}

				datas->emplace_back();
				auto& data = datas->back();
				const auto* charData = context.currentFont->getCharData(currentChar);

				if(charData) {
					currentPosition.add(Font::normalize(lastCharData->matrices.horiAdvance), 0);
					lastCharData = charData;
					data.region = &charData->region;
					data.fontColor = context.currentColor;

					Geom::Shape::OrthoRectFloat box = charData->charBox.as<float>();
					box.setSrc(Font::normalize(charData->matrices.horiBearingX), Font::normalize(charData->matrices.horiBearingY - charData->matrices.height));
					box.move(currentPosition.getX(), currentPosition.getY());
					box.scl(context.currentScale, context.currentScale);

					data.u0 = box.getSrcX();
					data.v0 = box.getSrcY();
					data.u1 = box.getEndX();
					data.v1 = box.getEndY();
				}
			}

			return datas;
		}

		std::shared_ptr<std::vector<GlyphVertData>> operator()(const std::string& text) const {
			if(
				const auto itr = layoutCache.find(text);
				itr != layoutCache.end()
			) {
				itr->second.frequency++;
				return itr->second.data;
			}

			return parse(text);
		}

		[[nodiscard]] GlyphParser() : GlyphParser(nullptr){

		}
	};

	GlyphParser* parser = nullptr;

	void loadParser(const FontFlags* const defFont) {
		parser = new GlyphParser{defFont};
	}
}
