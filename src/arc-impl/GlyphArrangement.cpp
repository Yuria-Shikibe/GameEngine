module;

module Font.GlyphArrangement;

import Graphic.Draw;
import RuntimeException;
import Math;

void Font::GlyphLayout::render() const {
	if(count <= 0 || this->lastText.empty()) return;
	for (auto& glyph : this->toRender | std::ranges::views::take(count)){
		Graphic::Draw::vert_monochromeAll(
			glyph.region->getData(), glyph.fontColor, Graphic::Draw::contextMixColor,
			glyph.u0 + bound.getSrcX() + offset.x, glyph.v0 + bound.getSrcY() + offset.y,
			glyph.region->u0,glyph.region->v0,
			glyph.u0 + bound.getSrcX() + offset.x, glyph.v1 + bound.getSrcY() + offset.y,
			glyph.region->u0,glyph.region->v1,
			glyph.u1 + bound.getSrcX() + offset.x, glyph.v1 + bound.getSrcY() + offset.y,
			glyph.region->u1,glyph.region->v1,
			glyph.u1 + bound.getSrcX() + offset.x, glyph.v0 + bound.getSrcY() + offset.y,
			glyph.region->u1,glyph.region->v0
		);
	}
}

void Font::GlyphLayout::render(float progress) const {
	if(count <= 0 || this->lastText.empty()) return;
	progress = Math::clamp(progress);
	for (auto& glyph : this->toRender | std::ranges::views::take(static_cast<size_t>(progress * static_cast<float>(count)))){
		Graphic::Draw::vert_monochromeAll(
			glyph.region->getData(), glyph.fontColor, Graphic::Draw::contextMixColor,
			glyph.u0 + bound.getSrcX() + offset.x, glyph.v0 + bound.getSrcY() + offset.y, glyph.region->u0,
			glyph.region->v0,
			glyph.u0 + bound.getSrcX() + offset.x, glyph.v1 + bound.getSrcY() + offset.y, glyph.region->u0,
			glyph.region->v1,
			glyph.u1 + bound.getSrcX() + offset.x, glyph.v1 + bound.getSrcY() + offset.y, glyph.region->u1,
			glyph.region->v1,
			glyph.u1 + bound.getSrcX() + offset.x, glyph.v0 + bound.getSrcY() + offset.y, glyph.region->u1,
			glyph.region->v0
		);
	}
}

Font::TypesettingTable::TypesettingTable(const FontFlags* const font): defaultFont(font), currentFont(font),
	fallbackFont(font) {
	if(!currentFont) throw ext::NullPointerException{};
	spaceSpaceing    = currentFont->data->spaceSpacing;
	lineSpacing      = currentFont->data->lineSpacingMin * 1.8f;
	paragraphSpacing = lineSpacing * 1.1f;
}

void Font::TokenParser::parse(const Font::TextView token, const ModifierableData& data) const {
	const auto hasType = token.find('#');

	if(hasType != Font::TextView::npos) {
		if(const auto itr = modifier.find(token.substr(0, hasType)); itr != modifier.end()) {
			itr->second(token.substr(hasType + 1), data);
		}
	} else {
		if(const auto itr = modifier.find(token); itr != modifier.end()) {
			itr->second(token, data);
		}
	}
}

void Font::GlyphParser::parse(const std::shared_ptr<GlyphLayout>& layout) const {
	constexpr auto npos = Font::TextString::npos;

	context.reset();
	layout->clear();
	auto& datas = layout->toRender;
	datas.resize(std::max(layout->lastText.size(), datas.capacity()));

	const Font::CharData* lastCharData = &Font::emptyCharData;
	bool tokenState                              = false;
	size_t tokenBegin                            = npos;
	Geom::Vec2 currentPosition{ 0, -context.lineSpacing };

	if(layout->lastText.empty()) {
		layout->bound.set(0, 0, 0, 0);
		return;
	}

	const auto totalSize = layout->lastText.size();
	int count = 0;
	for(int index = 0; index < totalSize; ++index) {
		const char currentChar = layout->lastText.at(index);
		//Token Check
		if(currentChar == TokenSignal) {
			if(tokenState) tokenState = false;
			else tokenState           = true;

			continue;
		}

		//Get Token
		if(tokenState) {
			if(currentChar == TokenSignal) {
				tokenState = false;
				goto process;
			}

			if(currentChar == TokenBeginCode) {
				tokenBegin = index + 1;
			} else if(currentChar == TokenEndCode) {
				if(tokenBegin != npos) {
					tokenParser->parse(
						layout->lastText.substr(tokenBegin, index - tokenBegin),
						{ context, currentPosition, lastCharData, *layout }
					);
				}
				tokenState = false;
			}

			continue;
		}

		//Process Glyph
	process:

		if(charParser->contains(currentChar)) {
			charParser->parse(currentChar, { context, currentPosition, lastCharData, *layout });
			continue;
		}

		//TODO check currentChar to adapt to UTF characters
		unsigned int charCode = static_cast<unsigned char>(currentChar);
		if(
			const int charCodeLength = ext::getCharCodeLength(currentChar);
			charCodeLength > 1 && index + charCodeLength <= totalSize
		){
			charCode = *reinterpret_cast<unsigned*>(&layout->lastText.at(index));
		}

		// datas.emplace_back();
		const auto* charData = context.currentFont->getCharData(charCode);

		if(charData) {
			auto& data = datas.at(count++);

			if(currentPosition.x + normalize(lastCharData->matrices.width + lastCharData->matrices.horiBearingX) *
			   context.currentScale > layout->maxWidth) {
				currentPosition.x = layout->maxWidth - normalize(lastCharData->matrices.horiAdvance) * context.
				                    currentScale;
				ParserFunctions::endLine({ context, currentPosition, lastCharData, *layout });
			}

			lastCharData = charData;
			data.region  = &charData->region;
			if(!data.region) {
				throw ext::NullPointerException{ "Null TextureRegion" };
			}
			data.fontColor = context.currentColor;

			Geom::OrthoRectFloat box = charData->charBox.as<float>();


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

	layout->count = count;

	if(layout->lastText.back() != '\n' && charParser->contains('\n')) {
		charParser->parse('\n', { context, currentPosition, lastCharData, *layout });
	}

	layout->bound.addSize(0, -2 * context.currentLineBound.getHeight());

	layout->bound.setLargerWidth(currentPosition.getX() + normalize(lastCharData->matrices.horiAdvance));
	layout->bound.setWidth(std::min(layout->bound.getWidth(), layout->maxWidth));

	//TODO should this really work?
	float offsetY = layout->bound.getHeight() + context.currentLineBound.getHeight() * 0.255f;

	std::for_each_n(std::execution::par_unseq, datas.begin(), layout->lastText.size(), [offsetY](GlyphVertData& data) {
		data.moveY(offsetY);
	});
}

void Font::initParser(const FontFlags* const defFont) {
	glyphParser = new GlyphParser{ defFont };

	glyphParser->charParser->modifier[' '] = [](const ModifierableData& data) {
		data.cursorPos.add(data.context.spaceSpaceing * data.context.currentScale, 0);
	};

	glyphParser->charParser->modifier['\n'] = [](const ModifierableData& data) {
		ParserFunctions::endLine(data);
	};

	glyphParser->tokenParser->modifier["color"] = [](const Font::TextView command, const ModifierableData& data) {
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

	glyphParser->tokenParser->modifier["font"] = [](const Font::TextView command, const ModifierableData& data) {
		if(command.front() == '[' && command.back() == ']') {
			if(const Font::TextView sub = command.substr(1, command.size() - 2); sub.empty()) {
				data.context.set(data.context.fallbackFont);
			} else {
				data.context.fallbackFont = data.context.currentFont;
				try {
					data.context.currentFont = glyphParser->fontLib->obtain(std::stoi(static_cast<Font::TextString>(sub)));
				} catch(std::invalid_argument& e) {
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
				static_cast<float>(data.context.currentFont->height) - static_cast<float>(data.context.fallbackFont->
					height), data.context.additionalYOffset);
			data.context.set(data.context.currentFont);
			data.context.currentLineBound.setLargerHeight(data.context.lineSpacing);
		}
	};

	//igl for Ignore Line, used for a line for token.
	glyphParser->tokenParser->modifier["ig"] = [](const Font::TextView command, const ModifierableData& data) {
		data.context.endlineOperation.push_back([](const ModifierableData& d) {
			d.context.currentLineBound.setHeight(0);
		});
	};

	glyphParser->tokenParser->modifier["tab"] = [](const Font::TextView command, const ModifierableData& data) {
		data.cursorPos.add(30, 0);
	};

	glyphParser->tokenParser->modifier["scl"] = [](const Font::TextView command, const ModifierableData& data) {
		if(command.front() == '[' && command.back() == ']') {
			if(const Font::TextView sub = command.substr(1, command.size() - 2); !sub.empty()) {
				float scl = 1.0f;
				try{
					scl = std::stof(static_cast<Font::TextString>(sub));
				} catch(std::invalid_argument& e) {

				}

				ParserFunctions::setScl(data, scl);
				return;
			}
		}

		ParserFunctions::resetScl(data);
	};

	glyphParser->tokenParser->modifier["off"] = [](const Font::TextView command, const ModifierableData& data) {
		if(command.front() == '[' && command.back() == ']') {
			if(const Font::TextView sub = command.substr(1, command.size() - 2); sub.empty()) {
				data.context.offset.setZero();
			} else {
				const auto splitIndex = sub.find_first_of(',');

				try {
					const float moveX = std::stof(static_cast<Font::TextString>(sub.substr(0, splitIndex)));
					const float moveY = std::stof(static_cast<Font::TextString>(sub.substr(splitIndex + 1)));

					data.context.offset.set(moveX, moveY);
				} catch(std::invalid_argument& e) {
					//TODO maybe ?
				}
			}
		} else {
			if(std::tolower(command.front()) == 'x') {
				try {
					data.context.offset.set(std::stof(static_cast<Font::TextString>(command.substr(1))), 0);
				} catch(std::invalid_argument& e) {
					//TODO maybe ?
				}
			} else if(std::tolower(command.front()) == 'y') {
				try {
					data.context.offset.set(0, std::stof(static_cast<Font::TextString>(command.substr(1))));
				} catch(std::invalid_argument& e) {
					//TODO maybe ?
				}
			}
		}
	};

	glyphParser->tokenParser->modifier["alp"] = [](const Font::TextView command, const ModifierableData& data) {
		if(command.front() == '[' && command.back() == ']') {
			if(const auto sub = command.substr(1, command.size() - 2); !sub.empty()) {
				float alpha = 1.0f;
				try { alpha = std::stof(static_cast<Font::TextString>(sub)); } catch(std::invalid_argument& e) {
				}

				data.context.currentColor.setA(alpha);
				return;
			}
		}

		data.context.currentColor.setA(1.0f);
	};

	//SuperScript Begin
	glyphParser->tokenParser->modifier["sut"] = [](const Font::TextView command, const ModifierableData& data) {
		ParserFunctions::setScl(data, 0.5f);
		data.context.offset.set(-normalize(data.charData->matrices.horiAdvance) * 0.05f,
		                        normalize(data.charData->matrices.horiBearingY + 45));
	};

	//SuperScript End
	glyphParser->tokenParser->modifier["\\sut"] = [](const Font::TextView command, const ModifierableData& data) {
		ParserFunctions::resetScl(data);
		data.context.offset.setZero();
	};

	//SubScript Begin
	glyphParser->tokenParser->modifier["sbt"] = [](const Font::TextView command, const ModifierableData& data) {
		ParserFunctions::setScl(data, 0.5f);
		data.context.offset.set(-normalize(data.charData->matrices.horiAdvance) * 0.05f,
		                        -normalize(
			                        -data.charData->matrices.horiBearingY + data.charData->matrices.height + 65));
	};

	//SubScript End
	glyphParser->tokenParser->modifier["\\sbt"] = [](const Font::TextView command, const ModifierableData& data) {
		ParserFunctions::resetScl(data);
		data.context.offset.setZero();
	};

	//Script End
	glyphParser->tokenParser->modifier["\\spt"] = [](const Font::TextView command, const ModifierableData& data) {
		ParserFunctions::resetScl(data);
		data.context.offset.setZero();
	};

	glyphParser->tokenParser->modifier["rst"] = [](const Font::TextView command, const ModifierableData& data) {
		data.context.currentColor = Graphic::Colors::WHITE;

		ParserFunctions::resetScl(data);
	};
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
		data.cursorPos.getX() + normalize(data.charData->matrices.horiAdvance) * data.context.currentScale
	);

	data.charData = &Font::emptyCharData;
	data.cursorPos.setX(0);

	data.context.currentLineBound.setHeight(data.context.lineSpacing * data.context.currentScale);

	for(const auto& operation : data.context.endlineOperation) {
		operation(data);
	}

	data.context.currentLineBound.addSize(0, data.context.additionalYOffset);

	if(data.context.additionalYOffset != 0.0f)
		std::for_each(std::execution::par_unseq, data.context.currentLineData.begin(),
		              data.context.currentLineData.end(),
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
