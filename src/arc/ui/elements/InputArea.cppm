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
import ext.Concepts;

export namespace UI{
	struct TextCursorDrawer;

	struct TextCaretData{
		/** @brief for rows, y for columns*/
		Font::TextLayoutPos beginPos{};
		Font::TextLayoutPos endPos{};

		//TODO necessary?
		Graphic::Color caretColor{Graphic::Colors::WHITE};
		Graphic::Color selectionColor{Graphic::Colors::ROYAL};

		[[nodiscard]] TextCaretData() = default;

		[[nodiscard]] TextCaretData(const Font::TextLayoutPos beginPos, const Font::TextLayoutPos endPos)
			: beginPos{beginPos},
			  endPos{endPos}{}
	};

	struct TextCaret : TextCaretData{
	private:
		static bool caresAbout(const Font::CharCode code){
			return std::isalnum(code) || code == Font::GlyphParser::TokenSignal;
		}
	public:
		/** THIS MUST BE IN A CONTINIOUS SEQ*/
		const Font::GlyphDrawData* dataBegin{};
		const Font::GlyphDrawData* dataEnd{};
		Math::Section<int> textSubrange{};

		std::shared_ptr<Font::GlyphLayout> layout{};

		//TODO should this thing here or in the area class?
		// TextCursorDrawer* drawer{};
		// bool insertMode{};

		using SeqType = std::vector<Font::GlyphDrawData>;

		explicit operator TextCaretData() const{
			return static_cast<TextCaretData>(*this);
		}

		[[nodiscard]] TextCaret() = default;

		[[nodiscard]] explicit TextCaret(const TextCaretData& data) : TextCaretData{data}{};

		[[nodiscard]] TextCaret(const Font::TextLayoutPos beginPos, const Font::TextLayoutPos endPos,
		                        const std::shared_ptr<Font::GlyphLayout>& layout)
			: TextCaretData{beginPos, endPos},
			  layout{layout}{}

		[[nodiscard]] explicit TextCaret(const std::shared_ptr<Font::GlyphLayout>& layout)
			: layout{layout}{}

		void draw() const;

		void moveToHome(const Font::GlyphDrawData* TextCaret::* ptr){
			auto& p = this->*ptr;
			while(true){
				if(p->isBegin())break;
				p--;
				if(p->isEndRow()){
					p++;
					break;
				}
			}

			alignTo(ptr);
		}

		void moveToEnd(const Font::GlyphDrawData* TextCaret::* ptr){
			auto& p = this->*ptr;
			while(true){
				if(p->isEnd() || p->isEndRow())break;
				p++;
			}

			alignTo(ptr);
		}

		void moveToNextBreakpoint_Left(const Font::GlyphDrawData* TextCaret::* ptr){
			auto& p = this->*ptr;

			while (!p->isBegin() && !caresAbout(p[-1].code)) {
				p--;
			}
			while (!p->isBegin() && caresAbout(p[-1].code)) {
				p--;
			}

			alignTo(ptr);
		}

		void moveToNextBreakpoint_Right(const Font::GlyphDrawData* TextCaret::* ptr){
			auto& p = this->*ptr;

			while (!p->isEnd() && caresAbout(p->code)) {
				p++;
			}
			while (!p->isEnd() && !caresAbout(p->code) && p->code != '\n') {
				p++;
			}

			alignTo(ptr);
		}

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
			return textSubrange.ordered();
		}

		void alignPos(){
			if(dataBegin){beginPos = dataBegin->layoutPos;}
			if(dataEnd){endPos = dataEnd->layoutPos;}
		}

		void alignBegin(){
			if(dataBegin){beginPos = dataBegin->layoutPos;}
		}

		void alignEnd(){
			if(dataEnd){endPos = dataEnd->layoutPos;}
		}

		void alignTo(const Font::GlyphDrawData* TextCaret::* ptr){
			if(ptr == &TextCaret::dataBegin){
				alignBegin();
			}else{
				alignEnd();
			}
		}

		void refreshDataPtr(){
			dataBegin = layout->find(beginPos);
			dataEnd = layout->find(endPos);

			alignPos();

			textSubrange.from = dataBegin ? dataBegin->index : 0;
			textSubrange.to = dataEnd ? dataEnd->index : 0;
		}

		void alignToSentinal(const Font::GlyphDrawData* TextCaret::* ptr){
			dataBegin = dataEnd = this->*ptr;
			beginPos = endPos = (this->*ptr)->layoutPos;
		}

		void alignToDirection(const bool right){
			const auto [l, r] = Math::Section{dataBegin, dataEnd}.ordered();
			if(right){
				dataBegin = dataEnd = r;
				beginPos = endPos = r->layoutPos;
			}else{
				dataEnd = dataBegin = l;
				endPos = beginPos = l->layoutPos;
			}
		}

		bool incr(const Font::GlyphDrawData* TextCaret::* ptr){
			if((this->*ptr)->isEnd())return false;
			++(this->*ptr);
			return true;
		}

		bool decr(const Font::GlyphDrawData* TextCaret::* ptr){
			if((this->*ptr)->isBegin())return false;
			--(this->*ptr);
			return true;
		}

		TextCaret& operator++(){
			if(isRangeSelecting()){
				alignToDirection(true);
				return *this;
			}

			if(!incr(&TextCaret::dataEnd))return *this;

			alignToSentinal(&TextCaret::dataEnd);
			return *this;
		}

		/**
		 * @brief Do this at the tail of the text
		 */
		TextCaret& forceIncr(){
			endPos.x ++;
			beginPos = endPos;
			return *this;
		}

		TextCaret& operator--(){
			if(isRangeSelecting()){
				alignToDirection(false);
				return *this;
			}

			if(!decr(&TextCaret::dataBegin))return *this;

			alignToSentinal(&TextCaret::dataBegin);
			return *this;
		}

		TextCaret& tryGotoAboveRow(){
			Font::TextLayoutPos pos = endPos;
			if(pos.y == 0)return *this;

			pos.y--;
			if(auto* rst = layout->find(pos)){
				if(rst->getRow() == beginPos.y){
					rst--;
				}

				this->dataEnd = rst;
				alignEnd();
			}

			return *this;
		}

		TextCaret& tryGotoBelowRow(){
			Font::TextLayoutPos pos = endPos;
			pos.y++;

			if(auto* rst = layout->find(pos)){
				this->dataEnd = rst;
				alignEnd();
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
	 * TODO ctrl + Delete / Backspace
	 * TODO IME context support
	 * TODO better do/undo
	 * TODO multiple carets
	 * TODO bound checks
	 * TODO token highlight parser
	 *
	 */
	class InputArea : public UI::TextWidget, public OS::TextInputListener{
	public:
		struct SnapshotData{ //TODO this is really a violent way to do/undo
			std::string text{};
			std::vector<TextCaretData> carets{};
		};

	protected:
		int maxSnapshotSize{64};
		int maxTextByteLength{32};

		std::deque<SnapshotData> snapshots{};
		int currentSnapshotIndex{0};

		Font::TextString hintText{""};
		bool showingHintText{false};
		float insertLineTimer{};

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

			for (TextCaret& caret : carets){
				caret.refreshDataPtr();
			}
		}

		bool isTextFocused() const;

		void setTextFocused();

		void setTextUnfocused() const;

		void genTextCaret(const std::optional<Font::TextLayoutPos> pos, const bool multi = false){
			genTextSection(pos, pos, multi);
		}

		void genTextSection(const std::optional<Font::TextLayoutPos> begin, const std::optional<Font::TextLayoutPos> end, const bool multi = false){
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
			getLastestSnapshot().carets = carets
			| std::ranges::views::transform(&TextCaret::operator TextCaretData)
			| std::ranges::to<decltype(getLastestSnapshot().carets)>();

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

		[[nodiscard]] std::optional<Font::TextLayoutPos> getLayoutPos(Geom::Vec2 caretPos) const {
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
			insertLineTimer = 0;
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

		void update(const Core::Tick delta) override {
			updateOperatrion();

			insertLineTimer += delta;
			if(constexpr float maxSpacing = 10000.0f; insertLineTimer > maxSpacing){
				insertLineTimer = Math::mod(insertLineTimer, maxSpacing);
			}

			flushInputBuffer();
			updateTextLayout();

			if(layoutChanged) {
				layout();
			}

			Elem::update(delta);
		}

		void updateOperatrion(){
			if(!isTextFocused())return;

			auto validKey = [this](const int key, const int mode){
				return keyDown(key, Ctrl::Act::Press, mode) || keyDown(key, Ctrl::Act::Repeat, mode);
			};

			auto act = [this](Concepts::Invokable<void(TextCaret&)> auto&& func){
				for(TextCaret& caret : carets){
					func(caret);
				}
				resetTime();
			};

			if(keyDown(Ctrl::Key::Home, Ctrl::Act::Press, Ctrl::Mode::None)){
				act([](TextCaret& caret){
					caret.moveToHome(&TextCaret::dataEnd);
					caret.alignToDirection(false);
				});
			}

			if(keyDown(Ctrl::Key::End, Ctrl::Act::Press, Ctrl::Mode::None)){
				act([](TextCaret& caret){
					caret.moveToEnd(&TextCaret::dataEnd);
					caret.alignToDirection(true);
				});
			}

			if(keyDown(Ctrl::Key::Home, Ctrl::Act::Press, Ctrl::Mode::Shift)){
				act([](TextCaret& caret){
					caret.moveToHome(&TextCaret::dataEnd);
					caret.alignEnd();
				});
			}

			if(keyDown(Ctrl::Key::End, Ctrl::Act::Press, Ctrl::Mode::Shift)){
				act([](TextCaret& caret){
					caret.moveToEnd(&TextCaret::dataEnd);
					caret.alignEnd();
				});
			}

			if(validKey(Ctrl::Key::Right, Ctrl::Mode::None)){
				act([](TextCaret& caret){++caret;});

			}

			if(validKey(Ctrl::Key::Left, Ctrl::Mode::None)){
				act([](TextCaret& caret){--caret;});
			}

			if(validKey(Ctrl::Key::Down, Ctrl::Mode::None)){
				act([](TextCaret& caret){
					caret.tryGotoBelowRow();
					caret.alignToSentinal(&TextCaret::dataEnd);
				});
			}

			if(validKey(Ctrl::Key::Up, Ctrl::Mode::None)){
				act([](TextCaret& caret){
					caret.tryGotoAboveRow();
					caret.alignToSentinal(&TextCaret::dataEnd);
				});
			}

			if(validKey(Ctrl::Key::Down, Ctrl::Mode::Ctrl)){
				act([](TextCaret& caret){
					caret.tryGotoBelowRow();
					caret.alignToSentinal(&TextCaret::dataEnd);
				});
			}

			if(validKey(Ctrl::Key::Up, Ctrl::Mode::Ctrl)){
				act([](TextCaret& caret){
					caret.tryGotoAboveRow();
					caret.alignToSentinal(&TextCaret::dataEnd);
				});
			}

			if(validKey(Ctrl::Key::Down, Ctrl::Mode::Shift)){
				act([](TextCaret& caret){
					caret.tryGotoBelowRow();
				});
			}

			if(validKey(Ctrl::Key::Up, Ctrl::Mode::Shift)){
				act([](TextCaret& caret){
					caret.tryGotoAboveRow();
				});
			}

			if(validKey(Ctrl::Key::Down, Ctrl::Mode::Ctrl_Shift)){
				act([](TextCaret& caret){
					caret.tryGotoBelowRow();
				});
			}

			if(validKey(Ctrl::Key::Up, Ctrl::Mode::Ctrl_Shift)){
				act([](TextCaret& caret){
					caret.tryGotoAboveRow();
				});
			}

			if(validKey(Ctrl::Key::Left, Ctrl::Mode::Shift)){
				act([](TextCaret& caret){
					caret.decr(&TextCaret::dataEnd);
					caret.alignEnd();
				});
			}

			if(validKey(Ctrl::Key::Right, Ctrl::Mode::Shift)){
				act([](TextCaret& caret){
					caret.incr(&TextCaret::dataEnd);
					caret.alignEnd();
				});
			}

			if(validKey(Ctrl::Key::Left, Ctrl::Mode::Ctrl)){
				act([](TextCaret& caret){
					caret.moveToNextBreakpoint_Left(&TextCaret::dataEnd);
					caret.alignToDirection(false);
				});
			}

			if(validKey(Ctrl::Key::Right, Ctrl::Mode::Ctrl)){
				act([](TextCaret& caret){
					caret.moveToNextBreakpoint_Right(&TextCaret::dataEnd);
					caret.alignToDirection(true);
				});
			}

			if(validKey(Ctrl::Key::Left, Ctrl::Mode::Ctrl_Shift)){
				act([](TextCaret& caret){
					caret.moveToNextBreakpoint_Left(&TextCaret::dataEnd);
					caret.alignEnd();
				});
			}

			if(validKey(Ctrl::Key::Right, Ctrl::Mode::Ctrl_Shift)){
				act([](TextCaret& caret){
					caret.moveToNextBreakpoint_Right(&TextCaret::dataEnd);
					caret.alignEnd();
				});
			}
		}

		void layout() override{
			setTextUpdated();
			updateTextLayout();
			updateGlyphPosition();

			Elem::layout();
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

			for (TextCaret& caret : carets){
				auto [begin, end] = caret.getSectionIndex();
				if(begin != end){
					glyphLayout->lastText.replace(glyphLayout->lastText.begin() + begin, glyphLayout->lastText.begin() + end, view);
					caret.alignToSentinal(&TextCaret::dataBegin);
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
			replaceSnapshot();

			for (TextCaret& caret : carets){
				auto [posBegin, posEnd] = caret.getSectionIndex();
				if(posBegin != posEnd){
					glyphLayout->lastText.erase(glyphLayout->lastText.begin() + posBegin, glyphLayout->lastText.begin() + posEnd);
					textChanged = true;
					caret.alignToDirection(false);
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
			replaceSnapshot();

			for (TextCaret& caret : carets){
				auto [posBegin, posEnd] = caret.getSectionIndex();
				if(posBegin != posEnd){
					glyphLayout->lastText.erase(glyphLayout->lastText.begin() + posBegin, glyphLayout->lastText.begin() + posEnd);
					textChanged = true;
					caret.alignToDirection(false);
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
				replaceSnapshot();

				TextCaret& caret = carets.front();
				auto [posBegin, posEnd] = caret.getSectionIndex();
				if(posBegin != posEnd){
					auto str = caret.getSectionText();
					glyphLayout->lastText.erase(glyphLayout->lastText.begin() + posBegin, glyphLayout->lastText.begin() + posEnd);
					textChanged = true;
					caret.alignToDirection(false);
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

			this->carets.clear();
			this->carets.reserve(carets.size());
			std::ranges::transform(carets, std::back_inserter(this->carets), [this](const TextCaretData& data){
				TextCaret caret{data};
				caret.layout = glyphLayout;
				caret.refreshDataPtr();
				return caret;
			});

			updateTextLayout(true);
			resetTime();

			for (TextCaret& caret : this->carets){
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

			snapshots.emplace_front(glyphLayout->lastText, carets
			                        | std::ranges::views::transform(&TextCaret::operator TextCaretData)
			                        | std::ranges::to<decltype(getLastestSnapshot().carets)>());

			if(snapshots.front().carets.size() > 1){
				throw std::exception{"Should Never Happen, Currently"};
			}
		}

		void popSnapshot(){
			snapshots.pop_front();
		}

		void replaceSnapshot(){
			popSnapshot();
			takeSnapshot();
		}

		SnapshotData& getLastestSnapshot(){
			return snapshots.front();
		}

		CursorType getCursorType() const noexcept override{
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
