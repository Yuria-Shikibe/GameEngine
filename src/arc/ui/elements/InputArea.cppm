//
// Created by Matrix on 2024/3/31.
//

export module UI.InputArea;

export import UI.Elem;
export import UI.Flags;

import std;
import Font.GlyphArrangement;
import Geom.Vector2D;
import Graphic.Color;
import Math;
import Geom.QuadTreeBrief;
import OS.TextInputListener;

import Ctrl.Constants;

import Graphic.Draw;

import Encoding;

export namespace UI{
	struct TextCursorDrawer;
	struct TextCaret{
		/** x for rows, y for columns*/
		Geom::Point2U beginPos{};
		Geom::Point2U endPos{};

		bool insertMode{};

		Graphic::Color caretColor{Graphic::Colors::LIGHT_GRAY};
		Graphic::Color selectionColor{Graphic::Colors::ROYAL};

		//TODO should this thing here or in the area class?
		TextCursorDrawer* drawer{};

		using SeqType = std::vector<Font::GlyphDrawData>;
		/** THIS MUST BE IN A CONTINIOUS SEQ*/
		const Font::GlyphDrawData* dataBegin{};
		const Font::GlyphDrawData* dataEnd{};

		void draw() const;

		[[nodiscard]] bool contains(const Font::GlyphDrawData* data) const{
			return data >= dataBegin && data < dataEnd;
		}

		[[nodiscard]] bool isRangeSelecting() const{
			return dataBegin != dataEnd || beginPos != endPos;
		}

		void swapSentinal(){
			std::swap(beginPos, endPos);
			std::swap(dataBegin, dataEnd);
		}

		/** smooth caret support usage*/
		[[nodiscard]] Geom::Vec2 getDrawPos(const bool end = true) const{
			if(end){
				if(dataEnd){
					return dataEnd->getBoundSrc();
				}
			}else{
				if(dataBegin){
					return dataBegin->getBoundSrc();
				}
			}

			return {};
		}

		[[nodiscard]] float getHeight(const bool end = true) const{
			if(end){
				if(dataEnd){
					return dataEnd->getFullHeight();
				}
			}else{
				if(dataBegin){
					return dataBegin->getFullHeight();
				}
			}

			return 0;
		}

		[[nodiscard]] Math::Section<int> getStrIndex() const{
			return {dataBegin ? dataBegin->index : 0, dataEnd ? dataEnd->index : 0};
		}

		void alignPos(){
			if(dataBegin){beginPos = dataBegin->layoutPos;}
			if(dataEnd){endPos = dataEnd->layoutPos;}
		}

		void refreshDataPtr(const Font::GlyphLayout* layout){
			dataBegin = layout->find(beginPos);
			dataEnd = layout->find(endPos);

			alignPos();

			if(dataBegin > dataEnd){
				swapSentinal();
			}
		}

		/**
		 * @param toEnd true - Align to end | talse - align to begin
		 */
		void alignToSentinal(const bool toEnd){
			if(toEnd){
				dataBegin = dataEnd;
				beginPos = endPos = dataEnd->layoutPos;
			}else{
				dataEnd = dataBegin;
				endPos = beginPos = dataBegin->layoutPos;
			}
		}

		TextCaret& operator++(){
			if(isRangeSelecting()){
				alignToSentinal(true);
				return *this;
			}

			if(dataBegin->isEnd())return *this;

			++dataEnd;

			alignToSentinal(true);
			return *this;
		}

		TextCaret& forceIncr(){
			if(isRangeSelecting()){
				alignToSentinal(true);
				return *this;
			}

			if(dataBegin->isEnd()){
				endPos.x ++;
				beginPos.x ++;
				return *this;
			}

			++dataEnd;

			alignToSentinal(true);
			return *this;
		}

		TextCaret& operator--(){
			if(isRangeSelecting()){
				alignToSentinal(false);
				return *this;
			}

			if(dataBegin->isBegin())return *this;

			--dataBegin;

			alignToSentinal(false);
			return *this;
		}

		TextCaret& tryGotoAboveRow(const Font::GlyphLayout* layout){
			Geom::Point2U pos = beginPos;
			if(pos.y == 0)return *this;

			pos.y--;
			if(auto* rst = layout->find(pos)){
				if(rst->getRow() == beginPos.y){
					rst--;
				}

				this->dataBegin = rst;
				alignToSentinal(false);
			}

			return *this;
		}

		TextCaret& tryGotoBelowRow(const Font::GlyphLayout* layout){
			Geom::Point2U pos = beginPos;
			pos.y++;

			if(auto* rst = layout->find(pos)){
				this->dataBegin = rst;
				alignToSentinal(false);
			}

			return *this;
		}

		[[nodiscard]] bool valid() const{
			return dataBegin != nullptr;
		}
	};

	struct TextCursorDrawer{
		virtual ~TextCursorDrawer() = default;

		virtual void operator()(const TextCaret* caret) = 0;
	};

	void TextCaret::draw() const{
		drawer->operator()(this);
	}

	/*
	 * TODO shift + -> / <-
	 * TODO shift + up / down
	 * TODO ctrl + -> / <-
	 * TODO ctrl + shift + -> / <-
	 * TODO ctrl + Delete / Backspace
	 * TODO IME context support
	 * TODO clipboard support
	 * TODO hint text support
	 * TODO length limit
	 * TODO better do/undo
	 * TODO multiple carets
	 * TODO bound checks
	 * TODO token highlight parser
	 */
	class InputArea : public UI::Elem, public OS::TextInputListener{
	public:
		struct BacktrackingData{ //TODO this is really a violent way to do/undo
			std::string text{};
			std::vector<TextCaret> carets{};
		};

	protected:
		int maxSnapshotSize = 100;
		std::deque<BacktrackingData> snapshots{};
		int currentSnapshotIndex{0};


		static Geom::OrthoRectFloat getBound(const Font::GlyphDrawData& glyph){
			return glyph.getBound();
		}


		Font::TextString hintText{};
		bool showingHintText{false};
		float time{};

		const Font::GlyphParser* defParser = Font::forwardParser.get();
		std::vector<TextCaret> carets{};

		std::shared_ptr<Font::GlyphLayout> glyphLayout = Font::obtainLayoutPtr();

		bool textChanged = false;

		void updateTextLayout(const bool forceUpdate = false) {
			if(textChanged){
				if(glyphLayout->lastText.empty())glyphLayout->lastText.push_back('\n');
				takeSnapshot();
			}
			Font::defGlyphParser->parseWith(glyphLayout, usingGlyphWidth ? std::numeric_limits<float>::max() : getValidWidth(), textChanged || forceUpdate);
			glyphLayout->setAlign(textAlignMode);
			textChanged = false;

			if(usingGlyphHeight){
				setHeight(border.getHeight() + glyphLayout->bound.getHeight());
			}

			if(usingGlyphWidth){
				setWidth(border.getWidth() + glyphLayout->bound.getWidth());
			}
		}

		bool isTextFocused() const;

		void setTextFocused();

		void setTextUnfocused() const;

		void genTextCaret(const std::optional<Geom::Point2U> pos, const bool multi = false){
			genTextSection(pos, pos, multi);
		}

		void genTextSection(const std::optional<Geom::Point2U> begin, const std::optional<Geom::Point2U> end, const bool multi = false){
			if(!begin.has_value() || !end.has_value())return;

			setTextFocused();

			if(!multi)carets.clear();

			resetTime();

			auto& cursor = carets.emplace_back(begin.value(), end.value());
			getLastSnapshot().carets.push_back(cursor);
		}

		Geom::Vec2 lastMousePos{};

		Geom::Vec2 transToTreeSrc(Geom::Vec2 vec2) const {
			vec2 -= absoluteSrc + border.bot_lft();
			vec2.y -= getValidHeight() - glyphLayout->bound.getHeight();
			return vec2;
		}

		void textUpdated() {
			textChanged = true;
		}



	public:
		bool usingGlyphHeight = false;
		bool usingGlyphWidth = false;

		InputArea(){
			glyphLayout->lastText.reserve(400);

			inputListener.on<UI::MouseActionPress>([this](const UI::MouseActionPress& event) {
				lastMousePos = static_cast<Geom::Vec2>(event);

				// genTextCaret(getLayoutPos(static_cast<Geom::Vec2>(event)));
			});

			inputListener.on<UI::MouseActionDrag>([this](const UI::MouseActionDrag& event) {
				auto begin = getLayoutPos(event.begin);
				auto end = getLayoutPos(event.end);
				genTextSection(begin, end);
			});
		}

		void resetTime(){
			time = 0;
		}


		//TODO use std::optional instead of {}?
		[[nodiscard]] std::optional<Geom::Point2U> getLayoutPos(Geom::Vec2 caretPos) const {
			if(glyphLayout->empty()){
				return std::nullopt;
			}

			auto validView = glyphLayout->takeValid();
			caretPos = transToTreeSrc(caretPos);

			auto beforeGlyph =
				std::lower_bound(validView.begin(), validView.end(), caretPos, [this](const Font::GlyphDrawData& glyph, const Geom::Vec2 caret){
					const float heightDst = caret.y - glyph.getBoundSrc().y;

					return (heightDst < 0 || heightDst < glyph.getFullHeight()) && (heightDst < 0 || caret.x > glyph.src.x);
				});

			if(beforeGlyph != validView.begin()){
				return (--beforeGlyph)->layoutPos;
			}

			return std::nullopt;
		}

		Align::Mode textAlignMode{Align::Mode::top_left};

		Font::TextString getTextCopy() const {
			return glyphLayout->lastText;
		}

		Font::TextString getTextMove() const && {
			return std::move(glyphLayout->lastText);
		}

		Font::TextString& getTextRef() const{
			return glyphLayout->lastText;
		}

		Font::TextView getTextView() const {
			return glyphLayout->lastText;
		}

		void setText(const Font::TextView text) {
			getTextRef() = text;
			textUpdated();
		}

		void update(const float delta) override {
			updateOperatrion();

			time += delta;
			if(constexpr float maxSpacing = 10000.0f; time > maxSpacing){
				time = Math::mod(time, maxSpacing);
			}

			updateTextLayout();

			if(layoutChanged) {
				layout();
			}

			for (auto& caret : carets){
				caret.refreshDataPtr(glyphLayout.get());
			}

			Elem::update(delta);
		}

		void updateOperatrion(){
			if(!isTextFocused())return;
			if(
				keyDown(Ctrl::KEY_RIGHT, Ctrl::Act_Press, Ctrl::Mode_IGNORE) ||
				keyDown(Ctrl::KEY_RIGHT, Ctrl::Act_Repeat, Ctrl::Mode_IGNORE)
			){
				for(auto& caret : carets){
					++caret;
					resetTime();
				}
			}

			if(keyDown(Ctrl::KEY_LEFT, Ctrl::Act_Press, Ctrl::Mode_IGNORE) ||
				keyDown(Ctrl::KEY_LEFT, Ctrl::Act_Repeat, Ctrl::Mode_IGNORE)){
				for(auto& caret : carets){
					--caret;
					resetTime();
				}
			}

			if(keyDown(Ctrl::KEY_DOWN, Ctrl::Act_Press, Ctrl::Mode_IGNORE) ||
				keyDown(Ctrl::KEY_DOWN, Ctrl::Act_Repeat, Ctrl::Mode_IGNORE)){
				for (auto& caret : carets){
					caret.tryGotoBelowRow(glyphLayout.get());
					resetTime();
				}
			}

			if(keyDown(Ctrl::KEY_UP, Ctrl::Act_Press, Ctrl::Mode_IGNORE) ||
				keyDown(Ctrl::KEY_UP, Ctrl::Act_Repeat, Ctrl::Mode_IGNORE)){
				for (auto& caret : carets){
					caret.tryGotoAboveRow(glyphLayout.get());
					resetTime();
				}
			}
		}

		void layout() override{
			Elem::layout();

			updateTextLayout();
		}

		void drawContent() const override{
			Graphic::Draw::alpha();

			if(!glyphLayout->empty()){
				const Geom::Vec2 off = glyphLayout->getDrawOffset();

				for (const auto & caret : carets){
					unsigned curRow = caret.dataBegin ? caret.dataBegin->getRow() : std::numeric_limits<unsigned>::max();
					Geom::Vec2 sectionBegin{caret.getDrawPos(false)};
					Geom::Vec2 sectionEnd{};

					for(auto data : std::span{caret.dataBegin, caret.dataEnd}){
						if(sectionBegin.isNaN()){
							sectionBegin = data.getBoundSrc();
						}

						sectionEnd = data.getBoundEnd();

						if(curRow != data.getRow() || data.isEndRow()){
							curRow = data.getRow();
							Rect rect{};
							if(data.isEndRow()){
								curRow++;
								sectionEnd.y = data.getBoundEnd().y;
								sectionEnd.x = glyphLayout->bound.getWidth();
							}
							Graphic::Draw::color(caret.selectionColor, 0.65f);
							Graphic::Draw::rectOrtho(Graphic::Draw::defaultTexture, rect.setVert(sectionBegin + off, sectionEnd + off));
							sectionBegin.setNaN();
						}
					}

					Rect rect{};
					sectionEnd.x = caret.getDrawPos().x;
					Graphic::Draw::color(caret.selectionColor, 0.65f);
					Graphic::Draw::rectOrtho(Graphic::Draw::defaultTexture, rect.setVert(sectionBegin + off, sectionEnd + off));
				}

				for (auto& glyph : glyphLayout->getGlyphs()){
					Graphic::Draw::color(glyph.fontColor);
					Graphic::Draw::quad(
						glyph.region,
						glyph.v00() + off,
						glyph.v10() + off,
						glyph.v11() + off,
						glyph.v01() + off
					);

					Graphic::Draw::color(Graphic::Colors::RED_DUSK);
					Graphic::Draw::rectPoint(glyph.v10() + off, 4);
					Graphic::Draw::tint(Graphic::Colors::YELLOW, .35f);
					Graphic::Draw::Line::setLineStroke(1.25f);
					Graphic::Draw::Line::rectOrtho(glyph.getBound().move(off));
				}
			}


			Graphic::Draw::color();
			Graphic::Draw::Line::setLineStroke(2.0f);
			if(isTextFocused() && Math::cycleStep<75, 40>(time)){
				for (const auto & caret : carets){
					Geom::Vec2 src = caret.getDrawPos() + glyphLayout->offset;
					src.y -= glyphLayout->bound.getHeight();
					Graphic::Draw::Line::line(src, {src.x, src.y + caret.getHeight()});
				}
			}


			Graphic::Draw::tint(Graphic::Colors::RED, .65f);
			Graphic::Draw::Line::setLineStroke(1.25f);
			Graphic::Draw::Line::rectOrtho(glyphLayout->bound, true, glyphLayout->offset);
		}

		//TODO append buffer to insert range
		void informTextInput(const unsigned codepoint, int mods) override{
			auto buffer = ext::convertTo<char>(codepoint);
			int size = ext::getUnicodeLength<int>(buffer.front());

			// std::cout << ext::getUnicodeLength(buffer.front()) << buffer.data() << std::endl;

			if(size == 1){ //TODO non-ASCII support test when glyph is done
				for (auto& caret : carets){
					auto [begin, end] = caret.getStrIndex();
					if(begin != end){
						glyphLayout->lastText.replace(glyphLayout->lastText.begin() + begin, glyphLayout->lastText.begin() + end, buffer.data(), buffer.data() + size);
						caret.alignToSentinal(false);
					}else{
						glyphLayout->lastText.insert_range(glyphLayout->lastText.begin() + caret.getStrIndex().from, buffer | std::ranges::views::take(size));
						caret.forceIncr();
					}
				}

				textChanged = true;
			}
		}

		void informBackSpace(int mode) override{
			for (auto& caret : carets){
				auto [begin, end] = caret.getStrIndex();
				if(begin != end){
					glyphLayout->lastText.erase(glyphLayout->lastText.begin() + begin, glyphLayout->lastText.begin() + end);
					textChanged = true;
					caret.alignToSentinal(false);
				}else if(begin > 0){
					begin--;
					auto cur = glyphLayout->lastText.begin() + begin;
					auto src = ext::gotoUnicodeHead(cur++);

					--caret;

					glyphLayout->lastText.erase(src, cur);
					textChanged = true;
				}
			}
		}

		void informSelectAll() override{
			if(glyphLayout->empty())return;
			carets.resize(1);
			carets.front().dataBegin = glyphLayout->getGlyphs().data();
			carets.front().dataEnd = &glyphLayout->back();
			carets.front().alignPos();
		}

		void informDelete(int mode) override{
			for (auto& caret : carets){
				auto [begin, end] = caret.getStrIndex();
				if(begin != end){
					glyphLayout->lastText.erase(glyphLayout->lastText.begin() + begin, glyphLayout->lastText.begin() + end);
					textChanged = true;
					caret.alignToSentinal(false);
				}else if(!caret.dataBegin->isEnd()){
					auto cur = glyphLayout->lastText.begin() + begin;
					const int length = ext::getUnicodeLength<int>(*cur);
					glyphLayout->lastText.erase(cur, cur + length);
					textChanged = true;
				}
			}
		}

		void informEnter(int mods) override{
			informTextInput('\n', 0);
		}

		void informDo() override{
			if(currentSnapshotIndex < snapshots.size() - 1){
				gotoSnapShot(++currentSnapshotIndex);
			}
		}

		void informUndo() override{
			if(currentSnapshotIndex > 0){
				gotoSnapShot(--currentSnapshotIndex);
			}
		}

		void gotoSnapShot(const int index){
			const auto& [text, carets] = snapshots.at(index);
			glyphLayout->lastText = text;

			this->carets = carets;

			updateTextLayout(true);
			resetTime();
		}

		void takeSnapshot(){
			if(currentSnapshotIndex > 0){
				snapshots.erase(snapshots.begin(), snapshots.begin() + currentSnapshotIndex);
				currentSnapshotIndex = 0;
			}

			if(snapshots.size() >= maxSnapshotSize){
				snapshots.pop_back();
			}

			snapshots.emplace_front(glyphLayout->lastText, carets);
		}

		BacktrackingData& getLastSnapshot(){
			return snapshots.back();
		}

		void calAbsoluteSrc(Elem* parent) override {
			Elem::calAbsoluteSrc(parent);
			glyphLayout->offset.set(absoluteSrc.x, absoluteSrc.y + bound.getHeight()).add(Align::getOffsetOf(textAlignMode, border));
			// glyphLayout->offset.set(0, 300);
		}
	};
}
