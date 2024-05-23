//
// Created by Matrix on 2024/3/31.
//

export module UI.InputArea;

export import UI.TextWidget;
export import UI.Flags;

import std;
import Font.GlyphArrangement;
import Geom.Vector2D;
import Geom.Rect_Orthogonal;
import Graphic.Color;
import Math;
import OS.TextInputListener;

import OS.Ctrl.Bind.Constants;

import ext.Encoding;

export namespace UI{
	struct TextCursorDrawer;
	struct TextCaret{
		/** @brief for rows, y for columns*/
		Geom::Point2U beginPos{};
		Geom::Point2U endPos{};

		/** THIS MUST BE IN A CONTINIOUS SEQ*/
		const Font::GlyphDrawData* dataBegin{};
		const Font::GlyphDrawData* dataEnd{};
		Math::Section<int> textSubrange{};

		std::shared_ptr<Font::GlyphLayout> layout{};

		//TODO necessary?
		Graphic::Color caretColor{Graphic::Colors::WHITE};
		Graphic::Color selectionColor{Graphic::Colors::ROYAL};

		//TODO should this thing here or in the area class?
		// TextCursorDrawer* drawer{};
		// bool insertMode{};

		using SeqType = std::vector<Font::GlyphDrawData>;


		[[nodiscard]] TextCaret() = default;

		[[nodiscard]] TextCaret(const Geom::Point2U& beginPos, const Geom::Point2U& endPos,
		                        const std::shared_ptr<Font::GlyphLayout>& layout)
			: beginPos{beginPos},
			  endPos{endPos},
			  layout{layout}{}

		[[nodiscard]] explicit TextCaret(const std::shared_ptr<Font::GlyphLayout>& layout)
			: layout{layout}{}

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

		[[nodiscard]] std::string getSectionText() const{
			auto [posBegin, posEnd] = getSectionIndex();
			if(posBegin != posEnd){
				std::string str = layout->lastText.substr(posBegin, posEnd - posBegin);
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

			return Geom::SNAN2;
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

		[[nodiscard]] Math::Section<int> getSectionIndex() const{
			return textSubrange;
		}

		void alignPos(){
			if(dataBegin){beginPos = dataBegin->layoutPos;}
			if(dataEnd){endPos = dataEnd->layoutPos;}
		}

		void refreshDataPtr(){
			dataBegin = layout->find(beginPos);
			dataEnd = layout->find(endPos);

			alignPos();

			if(dataBegin > dataEnd){
				swapSentinal();
			}

			textSubrange.from = dataBegin ? dataBegin->index : 0;
			textSubrange.to = dataEnd ? dataEnd->index : 0;
		}

		/**
		 * @param toEnd true - Align to end | false - align to begin
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

		/**
		 * @brief Do this at the tail of the text
		 */
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

		TextCaret& tryGotoAboveRow(){
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

		TextCaret& tryGotoBelowRow(){
			Geom::Point2U pos = beginPos;
			pos.y++;

			if(auto* rst = layout->find(pos)){
				this->dataBegin = rst;
				alignToSentinal(false);
			}

			return *this;
		}

		[[nodiscard]] bool valid() const{
			return dataBegin != nullptr && dataEnd != nullptr;
		}
	};

	struct TextCursorDrawer{
		virtual ~TextCursorDrawer() = default;

		virtual void operator()(const TextCaret* caret) = 0;
	};

	void TextCaret::draw() const{
		// drawer->operator()(this);
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
	class InputArea : public UI::TextWidget, public OS::TextInputListener{
	public:
		struct SnapshotData{ //TODO this is really a violent way to do/undo
			std::string text{};
			std::vector<TextCaret> carets{};
		};

	protected:
		int maxSnapshotSize{64};
		int maxTextByteLength{32};

		std::deque<SnapshotData> snapshots{};
		int currentSnapshotIndex{0};

		Font::TextString hintText{""};
		bool showingHintText{false};
		float time{};

		std::vector<TextCaret> carets{};

		std::u32string inputBuffer{};

		[[nodiscard]] bool shouldShowHint() const noexcept{
			return !hintText.empty() && carets.empty() && (showingHintText || empty());
		}

		void enableHint(){
			if(showingHintText && getText() == hintText)return;
			showingHintText = true;
			setText(hintText);
		}

		void disableHint(){
			if(!showingHintText)return;
			showingHintText = false;
			setText("");
		}

		void updateTextLayout(const bool forceUpdate = false) {
			if(textChanged){
				if(glyphLayout->lastText.empty())glyphLayout->lastText.push_back('\n');
				takeSnapshot();
			}

			layoutText(forceUpdate);

			for (auto& caret : carets){
				caret.refreshDataPtr();
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

			carets.emplace_back(beginPos, endPos, glyphLayout);
			getLastestSnapshot().carets = carets;

			updateHint();
		}

		Geom::Vec2 transToTreeSrc(Geom::Vec2 vec2) const {
			vec2 -= absoluteSrc + border.bot_lft();
			vec2.y -= getValidHeight();
			vec2.div(glyphLayout->getDrawBound().getSize());
			vec2.y += 1;
			vec2.mul(glyphLayout->getRawBound().getSize());
			return vec2;
		}

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

		void updateHint(){
			if(shouldShowHint()){
				enableHint();
			}else{
				disableHint();
			}
		}
	public:

		InputArea(){
			defParser = Font::forwardParser.get();

			glyphLayout->lastText.reserve(400);

			inputListener.on<UI::MouseActionDrag>([this](const UI::MouseActionDrag& event) {
				const auto begin = getLayoutPos(event.begin);
				const auto end = getLayoutPos(event.end);
				genTextSection(begin, end);
			});

			touchbility = TouchbilityFlags::enabled;
		}

		constexpr void resetTime() noexcept{
			time = 0;
		}

		[[nodiscard]] constexpr int getMaxTextLength() const noexcept{ return maxTextByteLength; }

		constexpr void setMaxTextLength(const int maxTextLength) noexcept{
			this->maxTextByteLength = maxTextLength;
		}

		[[nodiscard]] bool isTextNearlyFull() const noexcept{
			static constexpr auto UTF8_CharSize = 4;
			return maxTextByteLength - getText().size() < UTF8_CharSize;
		}

		[[nodiscard]] bool isTextFull() const noexcept{
			return getText().size() == maxTextByteLength;
		}

		void setText(const Font::TextView text) {
			getTextRef() = text;
			setTextUpdated();
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

			Widget::update(delta);
		}

		void updateOperatrion(){
			if(!isTextFocused())return;
			if(
				keyDown(Ctrl::Key::Right, Ctrl::Act::Press, Ctrl::Mode::Ignore) ||
				keyDown(Ctrl::Key::Right, Ctrl::Act::Repeat, Ctrl::Mode::Ignore)
			){
				for(auto& caret : carets){
					++caret;
					resetTime();
				}
			}

			if(keyDown(Ctrl::Key::Left, Ctrl::Act::Press, Ctrl::Mode::Ignore) ||
				keyDown(Ctrl::Key::Left, Ctrl::Act::Repeat, Ctrl::Mode::Ignore)){
				for(auto& caret : carets){
					--caret;
					resetTime();
				}
			}

			if(keyDown(Ctrl::Key::Down, Ctrl::Act::Press, Ctrl::Mode::Ignore) ||
				keyDown(Ctrl::Key::Down, Ctrl::Act::Repeat, Ctrl::Mode::Ignore)){
				for (auto& caret : carets){
					caret.tryGotoBelowRow();
					resetTime();
				}
			}

			if(keyDown(Ctrl::Key::Up, Ctrl::Act::Press, Ctrl::Mode::Ignore) ||
				keyDown(Ctrl::Key::Up, Ctrl::Act::Repeat, Ctrl::Mode::Ignore)){
				for (auto& caret : carets){
					caret.tryGotoAboveRow();
					resetTime();
				}
			}
		}

		void layout() override{
			setTextUpdated();
			updateTextLayout();
			updateGlyphPosition();

			Widget::layout();
		}

		void drawContent() const override;

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
				auto [begin, end] = caret.getSectionIndex();
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
				auto [posBegin, posEnd] = caret.getSectionIndex();
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
				auto [posBegin, posEnd] = caret.getSectionIndex();
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

		void informEnter(const int mods) override{
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
				return carets.front().getSectionText();
			}else{
				//TODO multi section return
				return {};
			}
		}

		std::string getClipboardClip() override{
			if(carets.empty())return {};
			if(carets.size() == 1){
				auto& caret = carets.front();
				auto [posBegin, posEnd] = caret.getSectionIndex();
				if(posBegin != posEnd){
					auto str = caret.getSectionText();
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
				caret.refreshDataPtr();
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
				throw std::exception{"Should Never Happen"};
			}
		}

		SnapshotData& getLastestSnapshot(){
			return snapshots.front();
		}

		CursorType getCursorType() const override{
			if(touchbility != TouchbilityFlags::enabled){
				return tooltipbuilder ? CursorType::regular_tip : CursorType::regular;
			}

			return CursorType::textInput;
		}

		bool onEsc() override{
			if(carets.empty())return true;

			setTextUnfocused();
			carets.clear();

			updateHint();

			return false;
		}

		bool empty() const noexcept{
			return getText().size() <= 1;
		}

		void informEscape(unsigned int codepoint, int mods) override{
			onEsc();
		}

		[[nodiscard]] Font::TextString& getHintText(){ return hintText; }
		[[nodiscard]] const Font::TextString& getHintText() const { return hintText; }

		void setHintText(const Font::TextString& hintText){ this->hintText = hintText; }
		void setHintText(Font::TextString&& hintText){ this->hintText = std::move(hintText); }
		void setHintText(const Font::TextView hintText){ this->hintText = hintText; }
	};
}
