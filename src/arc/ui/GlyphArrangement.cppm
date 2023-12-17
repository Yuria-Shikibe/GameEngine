//
// Created by Matrix on 2023/11/30.
//

module;

export module GlyphArrangement;

import RuntimeException;
import Font;
import Graphic.Draw;
import Graphic.Color;
import Container.Pool;
import Geom.Shape.Rect_Orthogonal;
import GL.Texture.TextureRegionRect;

import Geom.Vector2D;

import Align;

import <vector>;
import <string>;
import <algorithm>;
import <iostream>;
import <execution>;
import <functional>;
import <memory>;
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
		std::vector<GlyphVertData> toRender{};

		float maxWidth{std::numeric_limits<float>::max()};

		Geom::Shape::OrthoRectFloat bound{};

		std::string last{};

		void reset() {
			toRender.clear();
			bound.setSize(0, 0);
			bound.setSrc(0, 0);
			offset.setZero();
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

		void render() const {
			std::ranges::for_each(toRender, [this](const GlyphVertData& glyph) {
				Graphic::Draw::vert_monochromeAll(
					glyph.region->getData(), glyph.fontColor, Graphic::Draw::contextMixColor,
					glyph.u0 + bound.getSrcX() + offset.x, glyph.v0 + bound.getSrcY() + offset.y, glyph.region->u0, glyph.region->v0,
					glyph.u0 + bound.getSrcX() + offset.x, glyph.v1 + bound.getSrcY() + offset.y, glyph.region->u0, glyph.region->v1,
					glyph.u1 + bound.getSrcX() + offset.x, glyph.v1 + bound.getSrcY() + offset.y, glyph.region->u1, glyph.region->v1,
					glyph.u1 + bound.getSrcX() + offset.x, glyph.v0 + bound.getSrcY() + offset.y, glyph.region->u1, glyph.region->v0
				);
			});
		}

		void render(const float progress) const {
			for(size_t i = 0; i < static_cast<size_t>(progress * static_cast<float>(toRender.size())); ++i) {
				const GlyphVertData& glyph = toRender.at(i);
				Graphic::Draw::vert_monochromeAll(
					glyph.region->getData(), glyph.fontColor, Graphic::Draw::contextMixColor,
					glyph.u0 + bound.getSrcX() + offset.x, glyph.v0 + bound.getSrcY() + offset.y, glyph.region->u0, glyph.region->v0,
					glyph.u0 + bound.getSrcX() + offset.x, glyph.v1 + bound.getSrcY() + offset.y, glyph.region->u0, glyph.region->v1,
					glyph.u1 + bound.getSrcX() + offset.x, glyph.v1 + bound.getSrcY() + offset.y, glyph.region->u1, glyph.region->v1,
					glyph.u1 + bound.getSrcX() + offset.x, glyph.v0 + bound.getSrcY() + offset.y, glyph.region->u1, glyph.region->v0
				);
			}
		}

		[[nodiscard]] GlyphLayout() = default;
	};

	//TODO cache support. maybe?
	struct GlyphLayoutCache {
		size_t frequency{0};

		std::shared_ptr<GlyphLayout> data{nullptr};
	};

	std::unordered_map<std::string, GlyphLayoutCache> layoutCache(MAX_CAHCE);

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

		[[nodiscard]] explicit TypesettingTable(const FontFlags* const font)
			: defaultFont(font), currentFont(font), fallbackFont(font) {
			if(!currentFont)throw ext::NullPointerException{};
			spaceSpaceing = currentFont->data->spaceSpacing;
			lineSpacing = currentFont->data->lineSpacingMin * 1.8f;
			paragraphSpacing = lineSpacing * 1.1f;
		}

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
		const Font::FontData::CharData*& charData;
		GlyphLayout& layout;

		[[nodiscard]] ModifierableData(TypesettingTable& context, Geom::Vector2D& vec2,
			const Font::FontData::CharData*& data, GlyphLayout& layout)
			: context(context),
			cursorPos(vec2),
			charData(data),
			layout(layout) {
		}
	};

namespace ParserFunctions {
	void setScl(const ModifierableData& data, const float target) {

		const float last = data.context.currentScale;

		data.context.currentScale = target;

		data.context.additionalYOffset = std::max(
			data.context.additionalYOffset,
			(data.context.currentScale - 1.0f) * data.context.currentLineBound.getHeight() * 0.75f
		);

		float yMove = 0.0f;

		if(data.cursorPos.x == 0.0f) {
			yMove = data.context.lineSpacing * (last - data.context.currentScale);
		}

		data.cursorPos.add(
		0, yMove
		);
	}

	void resetScl(const ModifierableData& data) {
		setScl(data, 1.0f);
	}

	void endLine(const ModifierableData& data) {
		data.context.currentLineBound.setLargerWidth(
			data.cursorPos.getX() + normalize(data.charData->matrices.horiAdvance) * data.context.currentScale
		);

		data.charData = &Font::emptyCharData;
		data.cursorPos.setX(0);

		data.context.currentLineBound.setHeight(data.context.lineSpacing * data.context.currentScale);

		for (const auto& operation : data.context.endlineOperation) {
			operation(data);
		}

		data.context.currentLineBound.addSize(0, data.context.additionalYOffset);

		if(data.context.additionalYOffset != 0.0f)std::for_each(std::execution::par_unseq, data.context.currentLineData.begin(), data.context.currentLineData.end(),
			[lineSpacing = data.context.additionalYOffset](GlyphVertData* vertData) {
			vertData->moveY(-lineSpacing);
		});

		data.cursorPos.add(0, -data.context.currentLineBound.getHeight());

		data.context.currentLineData.clear();
		data.layout.bound.setLargerWidth(data.context.currentLineBound.getWidth());
		data.layout.bound.setHeight(-data.cursorPos.getY() + data.context.currentLineBound.getHeight());

		data.context.currentLineBound.setSrc(0, 0);

		data.context.additionalYOffset = 0;

		data.context.endlineOperation.clear();
	}
}

	class TokenParser {
	public:
		virtual ~TokenParser() = default;

		std::unordered_map<std::string, std::function<void(const std::string&, const ModifierableData& data)>> modifier{};

		virtual void parse(const std::string& token, const ModifierableData& data) const {
			const int hasType = token.find('#');

			if(hasType != std::string::npos) {
				if(const auto itr = modifier.find(token.substr(0, hasType)); itr != modifier.end()) {
					itr->second(token.substr(hasType + 1), data);
				}
			}else {
				if(const auto itr = modifier.find(token); itr != modifier.end()) {
					itr->second(token, data);
				}
			}


		}

		void operator()(const std::string& token, const ModifierableData& data) const {
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

		mutable TypesettingTable context{};

		std::unique_ptr<TokenParser> tokenParser{std::make_unique<TokenParser>()};
		std::unique_ptr<CharParser> charParser{std::make_unique<CharParser>()};

		/**
		 * \brief Customizable Fields
		 */
		[[nodiscard]] explicit GlyphParser(const FontFlags* const defFont) : context{defFont}{

		}

		virtual void parse(std::shared_ptr<GlyphLayout> layout, const std::string_view text) const {
			constexpr auto npos = std::string::npos;

			if(layout->last == text)return;
			layout->last = text;

			context.reset();
			layout->reset();
			auto& datas = layout->toRender;
			datas.reserve(text.length());

			const Font::FontData::CharData* lastCharData = &Font::emptyCharData;
			bool tokenState = false;
			size_t tokenBegin = npos;
			std::string token{};
			Geom::Vector2D currentPosition{0, -context.lineSpacing};

			if(text.empty()) {
				layout->bound.set(0, 0,0 , 0);
				return;
			}
			for(size_t index = 0; index < text.size(); ++index) {
				const char currentChar = text.at(index);
				//Token Check
				if(currentChar == TokenSignal) {
					if(tokenState)tokenState = false;
					else tokenState = true;

					continue;
				}

				if(tokenState) {
					if(currentChar == TokenSignal) {
						tokenState = false;
						goto process;
					}

					if(currentChar == TokenBeginCode) {
						tokenBegin = index + 1;
					}else if(currentChar == TokenEndCode) {
						if(tokenBegin != npos) {
							token = text.substr(tokenBegin, index - tokenBegin);

							tokenParser->parse(token, {context, currentPosition, lastCharData, *layout});
						}
						tokenState = false;
					}

					continue;
				}

				process:

				if(charParser->contains(currentChar)) {
					charParser->parse(currentChar, {context, currentPosition, lastCharData, *layout});
					continue;
				}

				datas.emplace_back();
				auto& data = datas.back();
				const auto* charData = context.currentFont->getCharData(currentChar);

				if(charData) {
					if(currentPosition.x + normalize(lastCharData->matrices.width + lastCharData->matrices.horiBearingX) * context.currentScale > layout->maxWidth) {
						currentPosition.x = layout->maxWidth - normalize(lastCharData->matrices.horiAdvance) * context.currentScale;
						ParserFunctions::endLine({context, currentPosition, lastCharData, *layout});
					}

					lastCharData = charData;
					data.region = &charData->region;
					if(!data.region) {
						throw ext::NullPointerException{"Null TextureRegion"};
					}
					data.fontColor = context.currentColor;

					Geom::Shape::OrthoRectFloat box = charData->charBox.as<float>();


					box.setSrc(
						normalize(charData->matrices.horiBearingX) * context.currentScale,
						normalize(charData->matrices.horiBearingY - charData->matrices.height) * context.currentScale
					);

					box.move(currentPosition.getX(), currentPosition.getY());
					box.scl(context.currentScale, context.currentScale);

					data.u0 = box.getSrcX() + context.offset.x * context.currentScale;
					data.v0 = box.getSrcY() + context.offset.y * context.currentScale;
					data.u1 = box.getEndX() + context.offset.x * context.currentScale;
					data.v1 = box.getEndY() + context.offset.y * context.currentScale;

					context.currentLineData.push_back(&data);

					currentPosition.add(normalize(charData->matrices.horiAdvance) * context.currentScale, 0);
				}
			}

			if(text.back() != '\n' && charParser->contains('\n')){
				charParser->parse('\n', {context, currentPosition, lastCharData, *layout});
			}

			layout->bound.addSize(0, -2 * context.currentLineBound.getHeight());

			layout->bound.setLargerWidth(currentPosition.getX() + normalize(lastCharData->matrices.horiAdvance));
			layout->bound.setWidth(std::min(layout->bound.getWidth(), layout->maxWidth));

			//TODO should this really work?
			float offsetY = layout->bound.getHeight() + context.currentLineBound.getHeight() * 0.255f;

			std::for_each(std::execution::par_unseq, datas.begin(), datas.end(), [offsetY](GlyphVertData& data) {
				data.moveY(offsetY);
			});
		}

		[[nodiscard]] std::shared_ptr<GlyphLayout> parse(const std::string_view text) const {
			const auto layout = std::make_shared<GlyphLayout>();

			parse(layout, text);

			return layout;
		}

		std::shared_ptr<GlyphLayout> operator()(const std::string& text) const {
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

	void initParser(const FontFlags* const defFont) {
		glyphParser = new GlyphParser{defFont};

		glyphParser->charParser->modifier[' '] = [](const ModifierableData& data) {
			data.cursorPos.add(data.context.spaceSpaceing * data.context.currentScale, 0);
		};

		glyphParser->charParser->modifier['\n'] = [](const ModifierableData& data) {
			ParserFunctions::endLine(data);
		};

		glyphParser->tokenParser->modifier["color"] = [](const std::string& command, const ModifierableData& data) {
			if(command.front() == '[' && command.back() == ']' ) {
				if(std::string&& sub = command.substr(1, command.size() - 2); sub.empty()) {
					data.context.currentColor = data.context.fallbackColor;
				}else {
					data.context.fallbackColor = data.context.currentColor;
					data.context.currentColor = Graphic::Color::valueOf(sub);
				}
			}else {
				if(const auto itr = parserableColors.find(command); itr != parserableColors.end()) {
					data.context.fallbackColor = data.context.currentColor;
					data.context.currentColor = itr->second;
				}
			}
		};

		glyphParser->tokenParser->modifier["font"] = [](const std::string& command, const ModifierableData& data) {
			if(command.front() == '[' && command.back() == ']' ) {
				if(std::string&& sub = command.substr(1, command.size() - 2); sub.empty()) {
					data.context.set(data.context.fallbackFont);
				}else {
					data.context.fallbackFont = data.context.currentFont;
					try {
						data.context.currentFont = Font::defaultManager->obtain(std::stoi(sub));
					}catch(std::invalid_argument e) {
						//TODO maybe ?
					}
				}
			}else {
				if(const auto itr = parserableFonts.find(command); itr != parserableFonts.end()) {
					data.context.fallbackFont = data.context.currentFont;
					data.context.set(itr->second);
				}
			}

			if(data.context.currentFont != data.context.fallbackFont) {
				data.context.additionalYOffset = std::max(static_cast<float>(data.context.currentFont->height) - static_cast<float>(data.context.fallbackFont->height), data.context.additionalYOffset);
				data.context.set(data.context.currentFont);
				data.context.currentLineBound.setLargerHeight(data.context.lineSpacing);
			}
		};

		//igl for Ignore Line, used for a line for token.
		glyphParser->tokenParser->modifier["igl"] = [](const std::string& command, const ModifierableData& data) {
			data.context.endlineOperation.push_back([](const ModifierableData& d) {
				d.context.currentLineBound.setHeight(0);
			});
		};

		glyphParser->tokenParser->modifier["tab"] = [](const std::string& command, const ModifierableData& data) {
			data.cursorPos.add(30, 0);
		};

		glyphParser->tokenParser->modifier["scl"] = [](const std::string& command, const ModifierableData& data) {
			if(command.front() == '[' && command.back() == ']') {
				if(std::string&& sub = command.substr(1, command.size() - 2); !sub.empty()){
					float scl = 1.0f;
					try {scl = std::stof(sub);}catch(std::invalid_argument e) {}

					ParserFunctions::setScl(data, scl);
					return;
				}
			}

			ParserFunctions::resetScl(data);
		};

		glyphParser->tokenParser->modifier["off"] = [](const std::string& command, const ModifierableData& data) {
			if(command.front() == '[' && command.back() == ']' ) {
				if(std::string&& sub = command.substr(1, command.size() - 2); sub.empty()) {
					data.context.offset.setZero();
				}else {
					const auto splitIndex = sub.find_first_of(',');

					try {
						const float moveX = std::stof(sub.substr(0, splitIndex));
						const float moveY = std::stof(sub.substr(splitIndex + 1));

						data.context.offset.set(moveX, moveY);
					}catch(std::invalid_argument e) {
						//TODO maybe ?
					}
				}
			}else {
				if(std::tolower(command.front()) == 'x') {
					try {
						data.context.offset.set(std::stof(command.substr(1)), 0);
					}catch(std::invalid_argument e) {
						//TODO maybe ?
					}
				}else if(std::tolower(command.front()) == 'y') {
					try {
						data.context.offset.set(0, std::stof(command.substr(1)));
					}catch(std::invalid_argument e) {
						//TODO maybe ?
					}
				}
			}
		};

		glyphParser->tokenParser->modifier["alp"] = [](const std::string& command, const ModifierableData& data) {
			if(command.front() == '[' && command.back() == ']') {
				if(std::string&& sub = command.substr(1, command.size() - 2); !sub.empty()){
					float alpha = 1.0f;
					try {alpha = std::stof(sub);}catch(std::invalid_argument e) {}

					data.context.currentColor.setA(alpha);
					return;
				}
			}

			data.context.currentColor.setA(1.0f);
		};

		//SuperScript Begin
		glyphParser->tokenParser->modifier["sut"] = [](const std::string& command, const ModifierableData& data) {
			ParserFunctions::setScl(data, 0.5f);
			data.context.offset.set(-normalize(data.charData->matrices.horiAdvance) * 0.05f, normalize(data.charData->matrices.horiBearingY + 45));
		};

		//SuperScript End
		glyphParser->tokenParser->modifier["\\sut"] = [](const std::string& command, const ModifierableData& data) {
			ParserFunctions::resetScl(data);
			data.context.offset.setZero();
		};

		//SubScript Begin
		glyphParser->tokenParser->modifier["sbt"] = [](const std::string& command, const ModifierableData& data) {
			ParserFunctions::setScl(data, 0.5f);
			data.context.offset.set(-normalize(data.charData->matrices.horiAdvance) * 0.05f, -normalize(-data.charData->matrices.horiBearingY + data.charData->matrices.height + 65));
		};

		//SubScript End
		glyphParser->tokenParser->modifier["\\sbt"] = [](const std::string& command, const ModifierableData& data) {
			ParserFunctions::resetScl(data);
			data.context.offset.setZero();
		};

		//Script End
		glyphParser->tokenParser->modifier["\\spt"] = [](const std::string& command, const ModifierableData& data) {
			ParserFunctions::resetScl(data);
			data.context.offset.setZero();
		};

		glyphParser->tokenParser->modifier["rst"] = [](const std::string& command, const ModifierableData& data) {
			data.context.currentColor = Graphic::Colors::WHITE;

			ParserFunctions::resetScl(data);
		};
	}
}
