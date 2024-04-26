//
// Created by Matrix on 2024/3/31.
//

export module UI.InputArea;

export import UI.Widget;
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

import ext.Encoding;

export namespace UI{
	struct TextCursorDrawer;
	struct TextCaret{
		/** x for rows, y for columns*/
		Geom::Point2U beginPos{};
		Geom::Point2U endPos{};
		Math::Section<int> stringIndexSnapshot{};

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

		[[nodiscard]] std::string getSectionText(const Font::GlyphLayout& glyphLayout) const{
			auto [posBegin, posEnd] = getStrIndex();
			if(posBegin != posEnd){
				std::string str = glyphLayout.lastText.substr(posBegin, posEnd - posBegin);
				str.push_back(0);
				return str;
			}
			return {};
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
			return stringIndexSnapshot;
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

			stringIndexSnapshot.from = dataBegin ? dataBegin->index : 0;
			stringIndexSnapshot.to = dataEnd ? dataEnd->index : 0;
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

			endPos.x ++;
			beginPos.x ++;
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
	 * TODO hint text support
	 * TODO better do/undo
	 * TODO multiple carets
	 * TODO bound checks
	 * TODO token highlight parser
	 */
	class InputArea : public UI::Widget, public OS::TextInputListener{
	public:
		struct BacktrackingData{ //TODO this is really a violent way to do/undo
			std::string text{};
			std::vector<TextCaret> carets{};
		};

	protected:
		int maxSnapshotSize = 100;
		int maxTextByteLength{32};

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

		std::u32string inputBuffer{};

		void updateTextLayout(const bool forceUpdate = false) {
			if(textChanged){
				if(glyphLayout->lastText.empty())glyphLayout->lastText.push_back('\n');
				takeSnapshot();
			}

			defParser->parseWith(glyphLayout, usingGlyphWidth ? std::numeric_limits<float>::max() : getValidWidth(), textChanged || forceUpdate);
			glyphLayout->setAlign(textAlignMode);
			textChanged = false;

			if(usingGlyphHeight){
				setHeight(border.getHeight() + glyphLayout->getDrawBound().getHeight());
			}

			if(usingGlyphWidth){
				setWidth(border.getWidth() + glyphLayout->getDrawBound().getWidth());
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

			auto beginPos = begin.value();
			auto endPos = end.value();

			if(std::ranges::any_of(carets, [beginPos, endPos](const TextCaret& caret){
				return caret.beginPos == beginPos && caret.endPos == endPos;
			}))return;

			carets.emplace_back(beginPos, endPos);
			getLastestSnapshot().carets = carets;
		}

		Geom::Vec2 transToTreeSrc(Geom::Vec2 vec2) const {
			vec2 -= absoluteSrc + border.bot_lft();
			vec2.y -= getValidHeight();
			vec2.div(glyphLayout->getDrawBound().getSize());
			vec2.y += 1;
			vec2.mul(glyphLayout->getRawBound().getSize());
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

			inputListener.on<UI::MouseActionDrag>([this](const UI::MouseActionDrag& event) {
				const auto begin = getLayoutPos(event.begin);
				const auto end = getLayoutPos(event.end);
				genTextSection(begin, end);
			});

			touchbility = TouchbilityFlags::enabled;
		}

		void setWrap(const bool wrapX = true, const bool wrapY = true){
			usingGlyphHeight = wrapY;
			usingGlyphWidth = wrapX;
		}

		[[nodiscard]] std::shared_ptr<Font::GlyphLayout>& getGlyphLayout(){
			return glyphLayout;
		}

		void resetTime(){
			time = 0;
		}

		[[nodiscard]] int getMaxTextLength() const{ return maxTextByteLength; }

		void setMaxTextLength(const int maxTextLength){ this->maxTextByteLength = maxTextLength; }

		//TODO use std::optional instead of {}?
		[[nodiscard]] std::optional<Geom::Point2U> getLayoutPos(Geom::Vec2 caretPos) const {
			if(glyphLayout->empty()){
				return std::nullopt;
			}

			auto validView = glyphLayout->takeValid();
			caretPos = transToTreeSrc(caretPos);

			// float heightDst = glyphLayout->getRawBoundRef().getHeight();
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

			flushInputBuffer();
			updateTextLayout();

			if(layoutChanged) {
				layout();
			}

			for (auto& caret : carets){
				caret.refreshDataPtr(glyphLayout.get());
			}

			Widget::update(delta);
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
			Widget::layout();

			updateTextLayout();
		}

		void drawContent() const override{
			Graphic::Draw::alpha();

			if(!glyphLayout->empty()){
				const Geom::Vec2 off = glyphLayout->getDrawOffset();

				for (const auto & caret : carets){
					unsigned curRow = caret.dataBegin ? caret.dataBegin->getRow() : std::numeric_limits<unsigned>::max();
					Geom::Vec2 sectionBegin{caret.getDrawPos(false).scl(glyphLayout->getScale())};
					Geom::Vec2 sectionEnd{};

					for(auto data : std::span{caret.dataBegin, caret.dataEnd}){
						if(sectionBegin.isNaN()){
							sectionBegin = data.getBoundSrc().scl(glyphLayout->getScale());
						}

						sectionEnd = data.getBoundEnd().scl(glyphLayout->getScale());

						if(curRow != data.getRow() || data.isEndRow()){
							curRow = data.getRow();
							Rect rect{};
							if(data.isEndRow()){
								curRow++;
								sectionEnd.set(glyphLayout->getRawBound().getWidth(), data.getBoundEnd().y).scl(glyphLayout->getScale());
							}
							Graphic::Draw::color(caret.selectionColor, 0.65f);
							Graphic::Draw::rectOrtho(Graphic::Draw::getDefaultTexture(), rect.setVert(sectionBegin + off, sectionEnd + off));
							sectionBegin.setNaN();
						}
					}

					Rect rect{};
					sectionEnd.x = caret.getDrawPos().x * glyphLayout->getScale();
					Graphic::Draw::color(caret.selectionColor, 0.65f);
					Graphic::Draw::rectOrtho(Graphic::Draw::getDefaultTexture(), rect.setVert(sectionBegin + off, sectionEnd + off));
				}

				glyphLayout->render();
			}


			Graphic::Draw::color();
			Graphic::Draw::Line::setLineStroke(2.0f);
			if(isTextFocused() && Math::cycleStep<75, 40>(time)){
				for (const auto & caret : carets){
					Geom::Vec2 src = caret.getDrawPos() * glyphLayout->getScale() + glyphLayout->offset;
					src.y -= glyphLayout->getDrawBound().getHeight();
					Graphic::Draw::Line::line(src, {src.x, src.y + caret.getHeight() * glyphLayout->getScale()});
				}
			}

			Graphic::Draw::tint(Graphic::Colors::RED, .65f);
			Graphic::Draw::Line::setLineStroke(1.25f);
			Graphic::Draw::Line::rectOrtho(glyphLayout->getDrawBound(), true, glyphLayout->offset);
		}

		//TODO append buffer to insert range
		void informTextInput(const unsigned codepoint, int mods) override{
			inputBuffer.push_back(codepoint);
		}

		void flushInputBuffer(){
			if(inputBuffer.empty())return;

			std::string str{};
			str.reserve(inputBuffer.size() * 4);

			for(const auto value : inputBuffer){
				auto buffer = ext::convertTo<char>(value);
				int size = ext::getUnicodeLength<int>(buffer.front());
				str.append_range(buffer | std::ranges::views::take(size));
			}

			insertText(str);

			inputBuffer.clear();
		}

		void insertText(const Font::TextView text){
			if(text.empty())return;

			int remainSize = maxTextByteLength - static_cast<int>(glyphLayout->lastText.size());
			if(remainSize <= 0)return;

			auto view = text.substr(0, remainSize);

			const auto charBegin = ext::gotoUnicodeHead(view.end() - 1);
			if(const unsigned length = ext::getUnicodeLength(*charBegin); length != view.end() - charBegin){
				view = text.substr(0, remainSize - (view.end() - charBegin));
			}

			const auto forwardSize = std::ranges::count_if(text, ext::isUnicodeHead);

			for (auto& caret : carets){
				auto [begin, end] = caret.getStrIndex();
				if(begin != end){
					glyphLayout->lastText.replace(glyphLayout->lastText.begin() + begin, glyphLayout->lastText.begin() + end, view);
					caret.alignToSentinal(false);
				}else{
					glyphLayout->lastText.insert_range(glyphLayout->lastText.begin() + begin, view);
				}

				for(int i = 0; i < forwardSize; ++i){
					caret.forceIncr();
				}
			}

			textChanged = true;
		}

		void informBackSpace(int mode) override{
			for (auto& caret : carets){
				auto [posBegin, posEnd] = caret.getStrIndex();
				if(posBegin != posEnd){
					glyphLayout->lastText.erase(glyphLayout->lastText.begin() + posBegin, glyphLayout->lastText.begin() + posEnd);
					textChanged = true;
					caret.alignToSentinal(false);
				}else if(posBegin > 0){

					const auto cur = glyphLayout->lastText.begin() + posBegin;
					const auto src = ext::gotoUnicodeHead(glyphLayout->lastText.begin() + (posBegin - 1));

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
				auto [posBegin, posEnd] = caret.getStrIndex();
				if(posBegin != posEnd){
					glyphLayout->lastText.erase(glyphLayout->lastText.begin() + posBegin, glyphLayout->lastText.begin() + posEnd);
					textChanged = true;
					caret.alignToSentinal(false);
				}else if(!caret.dataBegin->isEnd()){
					auto cur = glyphLayout->lastText.begin() + posBegin;
					const int length = ext::getUnicodeLength<int>(*cur);
					glyphLayout->lastText.erase(cur, cur + length);
					textChanged = true;
				}
			}
		}

		void informEnter(int mods) override{
			if(!glyphLayout->lastText.ends_with('\n')){
				glyphLayout->lastText.push_back('\n');
			}
			informTextInput('\n', mods);
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

		void informClipboardPaste(const std::string_view str) override{
			insertText(str);
		}

		std::string getClipboardCopy() override{
			if(carets.empty())return {};
			if(carets.size() == 1){
				return carets.front().getSectionText(*glyphLayout);
			}else{
				//TODO multi section return
				return {};
			}
		}

		std::string getClipboardClip() override{
			if(carets.empty())return {};
			if(carets.size() == 1){
				auto& caret = carets.front();
				auto [posBegin, posEnd] = caret.getStrIndex();
				if(posBegin != posEnd){
					auto str = caret.getSectionText(*glyphLayout);
					glyphLayout->lastText.erase(glyphLayout->lastText.begin() + posBegin, glyphLayout->lastText.begin() + posEnd);
					textChanged = true;
					caret.alignToSentinal(false);
					return str;
				}
			}else{
				//TODO multi section clip return
			}

			return {};
		}

		void gotoSnapShot(const int index){
			const auto& [text, carets] = snapshots.at(index);
			glyphLayout->lastText = text;

			this->carets = carets;

			updateTextLayout(true);
			resetTime();

			for (auto& caret : this->carets){
				caret.refreshDataPtr(glyphLayout.get());
			}
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

			if(snapshots.front().carets.size() > 1){
				throw std::exception{"wtf"};
			}
		}

		BacktrackingData& getLastestSnapshot(){
			return snapshots.front();
		}

		void calAbsoluteSrc(Widget* parent) override {
			Widget::calAbsoluteSrc(parent);
			glyphLayout->offset.set(absoluteSrc.x, absoluteSrc.y + bound.getHeight()).add(Align::getOffsetOf(textAlignMode, border));
			// glyphLayout->offset.set(0, 300);
		}

		CursorType getCursorType() const override{
			return CursorType::textInput;
		}
	};
}
