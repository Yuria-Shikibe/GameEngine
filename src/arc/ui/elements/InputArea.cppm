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
		Geom::Point2U begin{};
		Geom::Point2U end{};

		bool insertMode{};

		Graphic::Color caretColor{Graphic::Colors::LIGHT_GRAY};
		Graphic::Color selectionColor{Graphic::Colors::ROYAL};

		//TODO should this thing here or in the area class?
		TextCursorDrawer* drawer{};
		/** THIS MUST BE IN A CONTINIOUS SEQ*/
		const Font::GlyphDrawData* dataBegin{};
		const Font::GlyphDrawData* dataEnd{};

		void draw() const;

		[[nodiscard]] bool isRangeSelecting() const{
			return dataBegin != dataEnd || begin != end;
		}

		/** smooth caret support usage*/
		[[nodiscard]] Geom::Vec2 getDrawPos(const bool end = true) const{
			if(end){
				if(dataEnd){
					return dataEnd->getSrc();
				}
			}else{
				if(dataBegin){
					return dataBegin->getSrc();
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

		[[nodiscard]] int getStrIndex() const{
			return dataEnd ? dataEnd->index : 0;
		}

		void refreshDataPtr(const Font::GlyphLayout* layout){
			dataBegin = layout->find(begin);
			dataEnd = layout->find(end);
		}

		/**
		 * @param toEnd true - Align to end | talse - align to begin
		 */
		void alignToSentinal(const bool toEnd){
			if(toEnd){
				dataBegin = dataEnd;
				begin = end = dataEnd->layoutPos;
			}else{
				dataEnd = dataBegin;
				end = begin = dataBegin->layoutPos;
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
				end.x ++;
				begin.x ++;
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
			Geom::Point2U pos = begin;
			if(pos.y == 0)return *this;

			pos.y--;
			if(auto* rst = layout->find(pos)){
				if(rst->getRow() == begin.y){
					rst--;
				}

				this->dataBegin = rst;
				alignToSentinal(false);
			}

			return *this;
		}

		TextCaret& tryGotoBelowRow(const Font::GlyphLayout* layout){
			Geom::Point2U pos = begin;
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

	class InputArea : public UI::Elem, public OS::TextInputListener{
	public:
		struct BacktrackingData{
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
			if(textChanged)takeSnapshot();
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

		void genTextCursor(const std::optional<Geom::Point2U> pos, const bool multi = false){
			if(!pos.has_value())return;

			setTextFocused();

			if(!multi)carets.clear();

			resetTime();

			auto& cursor = carets.emplace_back(pos.value(), pos.value());
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

				genTextCursor(getLayoutPos(static_cast<Geom::Vec2>(event)));
			});
		}

		void resetTime(){
			time = 0;
		}

		unsigned getTextRows() const{
			return glyphLayout->empty() ? 0u : glyphLayout->takeValid().back().layoutPos.y;
		}

		float getAvgHeight() const{
			const unsigned int textRows = getTextRows();
			if(textRows){
				return glyphLayout->bound.getHeight() / static_cast<float>(textRows);
			}else{
				return 0;
			}
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
					const float heightDst = caret.y - glyph.getSrc().y;

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

			if(keyDown(Ctrl::KEY_ENTER, Ctrl::Act_Press, Ctrl::Mode_IGNORE) ||
				keyDown(Ctrl::KEY_ENTER, Ctrl::Act_Repeat, Ctrl::Mode_IGNORE)){
				informTextInput('\n', 0);
			}
		}

		void layout() override{
			Elem::layout();

			updateTextLayout();
		}

		void drawContent() const override{
			Graphic::Draw::alpha();
			glyphLayout->render();

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
		void informTextInput(unsigned codepoint, int mods) override{
			auto buffer = ext::convertTo<char>(codepoint);
			int size = ext::getUnicodeLength<int>(buffer.front());

			std::cout << ext::getUnicodeLength(buffer.front()) << buffer.data() << std::endl;

			if(size == 1){
				for (auto& caret : carets){
					glyphLayout->lastText.insert_range(glyphLayout->lastText.begin() + caret.getStrIndex(), buffer | std::ranges::views::take(size));
					caret.forceIncr();
				}

				textChanged = true;
			}
		}

		void informBackSpace() override{
			for (auto& caret : carets){
				int index = caret.getStrIndex();
				if(index > 0){
					index--;
					auto cur = glyphLayout->lastText.begin() + index;
					auto src = ext::gotoUnicodeHead(cur++);

					--caret;

					glyphLayout->lastText.erase(src, cur);
					textChanged = true;
				}
			}
		}

		void informDelete() override{

		}

		void snapBackTrace() override{
			if(currentSnapshotIndex < snapshots.size() - 1){
				gotoSnapShot(++currentSnapshotIndex);
			}
		}

		void snapForwardTrace() override{
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
