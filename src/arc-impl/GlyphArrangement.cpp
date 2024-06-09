module;

module Font.GlyphArrangement;

import Graphic.Draw;
import Graphic.Color;
import ext.RuntimeException;
import Math;


const Font::GlyphDrawData* Font::GlyphLayout::find(const TextLayoutPos layoutPos) const{
	auto itr = std::ranges::lower_bound(glyphs, layoutPos,
		[](const TextLayoutPos pos, const TextLayoutPos inputPos) -> bool {
		return (pos.y == inputPos.y && pos.x < inputPos.x) || pos.y < inputPos.y;
	}, &GlyphDrawData::layoutPos);

	if(itr != glyphs.end())return &itr.operator*();
	if(itr != glyphs.begin())return &(--itr).operator*();
	return nullptr;
}

void Font::GlyphLayout::render(const float alphaMask) const {
	if(empty()) return;
	const Geom::Vec2 off = getDrawOffset();
	for (auto& glyph : glyphs){
		Graphic::Draw::Overlay::color(glyph.fontColor, glyph.fontColor.a * alphaMask);
		Graphic::Draw::Overlay::Fill::quad(
			*glyph.region,
			glyph.v00().scl(scale) + off,
			glyph.v10().scl(scale) + off,
			glyph.v11().scl(scale) + off,
			glyph.v01().scl(scale) + off
		);
		//
		// Graphic::Draw::color(Graphic::Colors::RED_DUSK);
		// Graphic::Draw::rectPoint(glyph.v00().scl(getScale()) + off, 4);
		// Graphic::Draw::tint(Graphic::Colors::YELLOW, .35f);
		// Graphic::Draw::Line::setLineStroke(1.25f);
		// Graphic::Draw::Line::rectOrtho(glyph.getBound().scl(getScale(), getScale()).move(off));
	}

	// Graphic::Draw::Line::rectOrtho(drawBund.copy().setSrc(off));

}

void Font::GlyphLayout::render(const float alphaMask, float progress) const {
	if(empty()) return;
	progress = Math::clamp(progress);

	const Geom::Vec2 off = getDrawOffset();
	for (auto& glyph : this->glyphs | std::ranges::views::take(static_cast<size_t>(progress * static_cast<float>(glyphs.size())))){
		Graphic::Draw::Overlay::color(glyph.fontColor, glyph.fontColor.a * alphaMask);

		Graphic::Draw::Overlay::Fill::quad(
			*glyph.region,
			glyph.v00().scl(scale) + off,
			glyph.v10().scl(scale) + off,
			glyph.v11().scl(scale) + off,
			glyph.v01().scl(scale) + off
		);
	}
}

Font::TypesettingContext::TypesettingContext(const FontFace* const font): defaultFont(font), currentFont(font),
	fallbackFont(font) {
	if(!currentFont) throw ext::NullPointerException{};
	lineSpacing      = currentFont->data->lineSpacingDef * 1.8f;
	paragraphSpacing = lineSpacing * 1.1f;
}

void Font::TokenParser::parse(const unsigned curIndex, const Font::TextView token, const ModifierableData& data) const {
	const auto hasType = token.find('#');

	if(hasType != Font::TextView::npos) {
		if(const auto itr = modifier.find(token.substr(0, hasType)); itr != modifier.end()) {
			itr->second(curIndex, token.substr(hasType + 1), data);
		}else{
			if(fallBackModifier)fallBackModifier(curIndex, token, data);
		}
	} else {
		if(const auto itr = modifier.find(token); itr != modifier.end()) {
			itr->second(curIndex, token, data);
		}else{
			if(fallBackModifier)fallBackModifier(curIndex, token, data);
		}
	}
}

void Font::GlyphParser::parse(const std::shared_ptr<GlyphLayout>& layout) const {
	constexpr auto npos = Font::TextString::npos;

	context.reset();
	layout->clear();

	//The glyphs must have no address move during the parse
	auto& datas = layout->getGlyphs();
	datas.reserve(layout->lastText.size() + 1);

	const Font::CharData* lastCharData = &Font::emptyCharData;
	bool tokenState = false;
	size_t tokenBegin = npos;

	layout->getRawBound().set(0, 0, 0, 0);
	if(layout->lastText.empty()) {
		return;
	}

	Geom::Vec2 currentPosition{ 0, -context.lineSpacing };

	const auto totalSize = layout->lastText.size();
	for(unsigned int index = 0; index < totalSize; ++index) {
		const char currentChar = layout->lastText.at(index);
		//Token Check
		if(tokenParser){
			if(currentChar == TokenSignal){
				if(tokenState){
					if(tokenBegin != npos){
						index = tokenBegin - 1;
						tokenState = false;
						continue;
					}

					tokenState = false;
					goto process;
				}else{
					tokenState = true;
				}

				if(tokenParser->reserveTokenSentinal){
					goto process;
				}
				continue;
			}

			//Get Token
			if(tokenState) {
				if(currentChar == TokenBeginCode) {
					tokenBegin = index + 1;

					if(tokenParser->reserveTokenSentinal){
						goto process;
					}
				} else if(currentChar == TokenEndCode) {
					if(tokenBegin != npos) {
						tokenParser->parse(
							index,
							layout->lastText.substr(tokenBegin, index - tokenBegin), { context, currentPosition, lastCharData, *layout }
						);
					}
					tokenBegin = npos;
					tokenState = false;

					if(tokenParser->reserveTokenSentinal){
						goto process;
					}
				}else{
					if(tokenBegin == npos){
						tokenState = false;
						goto process;
					}else{
						if(index == totalSize - 1){
							index = tokenBegin - 1;
							tokenState = false;
						}
					}
				}

				continue;
			}
		}

		//Process Glyph
	process:

		//TODO check currentChar to adapt to UTF characters
		unsigned int charCode = static_cast<unsigned char>(currentChar);
		const int charCodeLength = ext::getUnicodeLength<int>(currentChar);

		if(charCodeLength > 1 && index + charCodeLength <= totalSize){
			charCode = ext::convertTo(layout->lastText.data() + index, charCodeLength);
		}

		const auto* charData = context.currentFont->getCharData(charCode);
		const bool hasCharToken = charParser->contains(charCode);

		if(hasCharToken){
			if(charParser->shouldNotContinue(charCode)){
				lastCharData = charData;
				ParserFunctions::pushData(charCode, static_cast<int>(index), charData, { context, currentPosition, lastCharData, *layout });
				charParser->parse(charCode, { context, currentPosition, lastCharData, *layout });
			}else{
				charParser->parse(charCode, { context, currentPosition, lastCharData, *layout });
			}
		}else{
			lastCharData = charData;
			ParserFunctions::pushData(charCode, static_cast<int>(index), charData, { context, currentPosition, lastCharData, *layout });
		}

		index += charCodeLength - 1;

	}

	if(datas.empty())return;

	if(!layout->back().isEndRow() && charParser->contains('\n')) {
		lastCharData = context.currentFont->getCharData('\n');
		ParserFunctions::pushData('\n', static_cast<int>(totalSize), lastCharData, { context, currentPosition, lastCharData, *layout });
		ParserFunctions::endLine({ context, currentPosition, lastCharData, *layout });
	}

	layout->getGlyphs().back().code = 0;

	layout->getRawBound().addSize(0, -2 * context.currentLineBound.getHeight());

	layout->getRawBound().setShorterWidth(layout->maxWidth);

	float offsetY = layout->getRawBound().getHeight() + layout->getGlyphs().front().getFullHeight() * 0.2f;

	std::for_each(std::execution::unseq, datas.begin(), datas.end(), [offsetY](GlyphDrawData& data) {
		data.moveY(offsetY);
	});

	layout->updateDrawbound();
}

void Font::initParser(const FontFace* const defFont) {
	defGlyphParser = std::make_unique<GlyphParser>(defFont);

	defGlyphParser->charParser->registerDefParser();

	defGlyphParser->tokenParser->modifier["color"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		if(command.front() == '[' && command.back() == ']') {
			if(const auto sub = command.substr(1, command.size() - 2); sub.empty()) {
				data.context.currentColor = data.context.fallbackColor;
			} else {
				data.context.fallbackColor = data.context.currentColor;
				data.context.currentColor  = Graphic::Color::valueOf(sub);
			}
		} else {
			if(const auto itr = parserableColors.find(command); itr != parserableColors.end()) {
				data.context.fallbackColor = data.context.currentColor;
				data.context.currentColor  = itr->second;
			}
		}
	};

	defGlyphParser->tokenParser->modifier["c"] = defGlyphParser->tokenParser->modifier.at("color");

	defGlyphParser->tokenParser->modifier["font"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		if(command.front() == '[' && command.back() == ']') {
			if(const Font::TextView sub = command.substr(1, command.size() - 2); sub.empty()) {
				data.context.set(data.context.fallbackFont);
			} else {
				data.context.fallbackFont = data.context.currentFont;
				try {
					data.context.currentFont = defGlyphParser->loadedFonts->obtain(std::stoi(static_cast<Font::TextString>(sub)));
				} catch([[maybe_unused]] std::invalid_argument& e) {
					//TODO throw maybe ?
				}
			}
		} else {
			if(const auto itr = parserableFonts.find(command); itr != parserableFonts.end()) {
				data.context.fallbackFont = data.context.currentFont;
				data.context.set(itr->second);
			}
		}

		if(data.context.currentFont != data.context.fallbackFont) {
			data.context.additionalYOffset = std::max(
				static_cast<float>(data.context.currentFont->size) - static_cast<float>(data.context.fallbackFont->
					size), data.context.additionalYOffset);
			data.context.set(data.context.currentFont);
			data.context.currentLineBound.setLargerHeight(data.context.lineSpacing);
		}
	};

	//igl for Ignore Line, used for a line for token.
	defGlyphParser->tokenParser->modifier["ig"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		data.context.endlineOperation.push_back([](const ModifierableData& d) {
			d.context.currentLineBound.setHeight(0);
		});
	};

	defGlyphParser->tokenParser->modifier["tab"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		data.cursorPos.add(30, 0);
	};

	defGlyphParser->tokenParser->modifier["scl"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		if(command.front() == '[' && command.back() == ']') {
			if(const Font::TextView sub = command.substr(1, command.size() - 2); !sub.empty()) {
				float scl = 1.0f;
				try{
					scl = std::stof(static_cast<Font::TextString>(sub));
				} catch([[maybe_unused]] std::invalid_argument& e) {

				}

				ParserFunctions::setScl(data, scl);
				return;
			}
		}

		ParserFunctions::resetScl(data);
	};

	defGlyphParser->tokenParser->modifier["off"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		if(command.front() == '[' && command.back() == ']') {
			if(const Font::TextView sub = command.substr(1, command.size() - 2); sub.empty()) {
				data.context.offset.setZero();
			} else {
				const auto splitIndex = sub.find_first_of(',');

				try {
					const float moveX = std::stof(static_cast<Font::TextString>(sub.substr(0, splitIndex)));
					const float moveY = std::stof(static_cast<Font::TextString>(sub.substr(splitIndex + 1)));

					data.context.offset.set(moveX, moveY);
				} catch([[maybe_unused]] std::invalid_argument& e) {
					//TODO maybe ?
				}
			}
		} else {
			if(std::tolower(command.front()) == 'x') {
				try {
					data.context.offset.set(std::stof(static_cast<Font::TextString>(command.substr(1))), 0);
				} catch([[maybe_unused]] std::invalid_argument& e) {
					//TODO maybe ?
				}
			} else if(std::tolower(command.front()) == 'y') {
				try {
					data.context.offset.set(0, std::stof(static_cast<Font::TextString>(command.substr(1))));
				} catch([[maybe_unused]] std::invalid_argument& e) {
					//TODO maybe ?
				}
			}
		}
	};

	defGlyphParser->tokenParser->modifier["alp"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		if(command.front() == '[' && command.back() == ']') {
			if(const auto sub = command.substr(1, command.size() - 2); !sub.empty()) {
				float alpha = 1.0f;
				try { alpha = std::stof(static_cast<Font::TextString>(sub)); } catch([[maybe_unused]] std::invalid_argument& e) {
				}

				data.context.currentColor.setA(alpha);
				return;
			}
		}

		data.context.currentColor.setA(1.0f);
	};

	//SuperScript Begin
	defGlyphParser->tokenParser->modifier["sup"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		ParserFunctions::setScl(data, 0.5f);
		data.context.offset.set(-normalizeLen(data.charData->glyphMatrices.horiAdvance) * 0.05f,
		                        normalizeLen(data.charData->glyphMatrices.horiBearingY + 45));
	};

	//SuperScript End
	defGlyphParser->tokenParser->modifier["\\sup"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		ParserFunctions::resetScl(data);
		data.context.offset.setZero();
	};

	//SubScript Begin
	defGlyphParser->tokenParser->modifier["sub"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		ParserFunctions::setScl(data, data.context.currentScale * 0.65f);

		data.context.offset.set(
			-normalizeLen(data.charData->glyphMatrices.horiAdvance) * 0.05f,
		    normalizeLen(data.charData->glyphMatrices.horiBearingY) - normalizeLen(data.charData->glyphMatrices.height) * 1.45f);
	};

	//SubScript End
	defGlyphParser->tokenParser->modifier["\\sub"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		ParserFunctions::resetScl(data);
		data.context.offset.setZero();
	};

	//Script End
	defGlyphParser->tokenParser->modifier["\\spt"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		ParserFunctions::resetScl(data);
		data.context.offset.setZero();
	};

	defGlyphParser->tokenParser->modifier["rst"] = [](unsigned, const Font::TextView command, const ModifierableData& data) {
		data.context.currentColor = Graphic::Colors::WHITE;

		ParserFunctions::resetScl(data);
	};

	using namespace Graphic::Colors;
	parserableColors.insert_or_assign("AQUA", AQUA);
	parserableColors.insert_or_assign("AQUA_SKY", AQUA_SKY);
	parserableColors.insert_or_assign("SKY_AQUA", AQUA_SKY);
	parserableColors.insert_or_assign("SKY", SKY);
	parserableColors.insert_or_assign("GRAY", GRAY);
	parserableColors.insert_or_assign("LIGHT_GRAY", LIGHT_GRAY);
	parserableColors.insert_or_assign("WHITE", WHITE);
	parserableColors.insert_or_assign("RED_DUST", RED_DUSK);
	parserableColors.insert_or_assign("PALE_GREEN", PALE_GREEN);
}

void Font::ParserFunctions::setScl(const ModifierableData& data, const float target) {
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

void Font::ParserFunctions::endLine(const ModifierableData& data) {
	data.context.currentLineBound.setLargerWidth(
		data.cursorPos.getX() + normalizeLen(data.charData->glyphMatrices.horiAdvance) * data.context.currentScale
	);

	data.charData = &Font::emptyCharData;

	data.context.currentLineBound.setHeight(data.context.lineSpacing * data.context.currentScale);

	for(const auto& operation : data.context.endlineOperation) {
		operation(data);
	}

	data.context.currentLineBound.addSize(0, data.context.additionalYOffset);

	const bool requiresLineSpacingCorrection  = data.context.additionalYOffset != 0.0f;

	float heightOffset = data.context.currentLineBound.getHeight() * 0.225f;

	for (const auto vertData : data.context.currentLineData){
		if(requiresLineSpacingCorrection){
			vertData->moveY(-data.context.additionalYOffset);
		}

		vertData->heightAlign.from = data.cursorPos.y - vertData->src.y - heightOffset;
		vertData->heightAlign.to = data.context.currentLineBound.getHeight() - (vertData->end.y - data.cursorPos.y) - vertData->heightAlign.from - heightOffset;
	}

	data.cursorPos.x = 0;

	data.cursorPos.y -= data.context.currentLineBound.getHeight();

	data.context.currentLineData.clear();
	data.layout.getRawBound().setLargerWidth(data.context.currentLineBound.getWidth());
	data.layout.getRawBound().setHeight(-data.cursorPos.getY() + data.context.currentLineBound.getHeight());

	data.context.currentLineBound.setSrc(0, 0);

	data.context.additionalYOffset = 0;

	data.context.currentLayoutPos.x = 0;
	data.context.currentLayoutPos.y++;

	data.context.endlineOperation.clear();
}

void Font::ParserFunctions::pushData(const CharCode code, const int index, const CharData* charData, const ModifierableData& data){
	static CharData tmpDummy{Font::emptyCharData};
	static const CharData* tmpDummyPtr = &tmpDummy;

	if(charData == nullptr){
		throw ext::NullPointerException{std::format("Null CharData with char code: {}",  ext::convertTo<char>(code))};
	}

	if(code != '\n' && (data.cursorPos.x + normalizeLen(charData->glyphMatrices.width + charData->glyphMatrices.horiBearingX) * data.context.currentScale) * data.layout.getScale() > data.layout.maxWidth){
		ParserFunctions::endLine({data.context, data.cursorPos, tmpDummyPtr, data.layout});
	}

	Geom::OrthoRectFloat box{};
	box.setSize(charData->getSize<float>());
	box.setSrc(
		normalizeLen(charData->glyphMatrices.horiBearingX) * data.context.currentScale,
		normalizeLen(charData->glyphMatrices.horiBearingY - charData->glyphMatrices.height) * data.context.currentScale
	);

	box.move(data.cursorPos.getX(), data.cursorPos.getY());
	box.sclSize(data.context.currentScale, data.context.currentScale);

	auto& glyphData = data.layout.getGlyphs().emplace_back(
		code,
		index,
		charData->region,
		data.context.currentColor,
		box.getSrc() + data.context.offset * data.context.currentScale,
		box.getEnd() + data.context.offset * data.context.currentScale,
		data.context.currentLayoutPos
	);

	if(!glyphData.region) {
		throw ext::NullPointerException{ "Null TextureRegion" };
	}

	data.context.currentLineData.push_back(&glyphData);
	data.context.currentLayoutPos.x++;

	data.cursorPos.add(normalizeLen(charData->glyphMatrices.horiAdvance) * data.context.currentScale, 0);
}
