module;

export module UI.Label;

import ext.RuntimeException;
import UI.Align;
import UI.Widget;
import Font.GlyphArrangement;
import std;

export namespace UI {
	class Label : public Widget {
	protected:
		std::shared_ptr<Font::GlyphLayout> glyphLayout{Font::obtainLayoutPtr()};

		Align::Mode textAlignMode{Align::Mode::top_left};
		std::function<Font::TextString()> textSource{};

		bool textChanged = false;

		void textUpdated() {
			textChanged = true;
		}

		void updateTextLayout() {
			float expectedWidth = getValidWidth();
			if(Math::zero(expectedWidth)){
				expectedWidth = std::numeric_limits<float>::max();
			}

			Font::defGlyphParser->parseWith(glyphLayout, usingGlyphWidth ? std::numeric_limits<float>::max() : expectedWidth, textChanged);
			glyphLayout->setAlign(Align::Mode::bottom_left);
			textChanged = false;
			if(usingGlyphHeight){
				setHeight(border.getHeight() + glyphLayout->getDrawBound().getHeight());
			}

			if(usingGlyphWidth){
				setWidth(border.getWidth() + glyphLayout->getDrawBound().getWidth());
			}
		}

		void updateGlyphPosition() const{
			const Rect drawBound = glyphLayout->getDrawBound();
			const auto offset = Align::getOffsetOf(textAlignMode, drawBound.getSize(), getValidBound());
			glyphLayout->offset.set(offset).add(absoluteSrc);
		}

	public:
		bool usingGlyphHeight = false;
		bool usingGlyphWidth = false;

		Label(){
			touchbility = TouchbilityFlags::disabled;
		}

		[[nodiscard]] std::shared_ptr<Font::GlyphLayout>& getGlyphLayout(){
			return glyphLayout;
		}

		[[nodiscard]] Font::TextView getLastText() const{
			return glyphLayout->lastText;
		}

		void setTextAlign(const Align::Mode align){
			textAlignMode = align;

			updateGlyphPosition();
		}

		void setLayoutDataPtr(const std::shared_ptr<Font::GlyphLayout>& layoutPtr) {
			glyphLayout = layoutPtr;
		}

		void setText(const Font::TextView text) {
			glyphLayout->lastText = text;
			textSource = nullptr;
			textUpdated();
		}

		void setText(Font::TextString&& text) {
			glyphLayout->lastText = std::move(text);
			textSource = nullptr;
			textUpdated();
		}

		void setText(const Font::TextView::const_pointer text) {
			glyphLayout->lastText = Font::TextView{text};
			textSource = nullptr;
			textUpdated();
		}

		void setText(Concepts::Invokable<Font::TextString()> auto&& charSource) {
			textSource = std::forward<decltype(charSource)>(charSource);
			glyphLayout->lastText = std::move(textSource());

			textUpdated();
		}

		void drawContent() const override;

		void calAbsoluteSrc(Widget* parent) override {
			Widget::calAbsoluteSrc(parent);
			updateGlyphPosition();
		}

		constexpr void setWrap(const bool wrapX = true, const bool height = true) noexcept{
			usingGlyphHeight = height;
			usingGlyphWidth = wrapX;
		}

		void setTextScl(const float scl = 1.0f) const{
			glyphLayout->setSCale(scl);
		}

		void update(float delta) override {
			if(textSource){
				Font::TextString cur = std::move(textSource());

				if(cur != getLastText()){
					glyphLayout->lastText = std::move(cur);
					textUpdated();
				}

				updateTextLayout();
			}else if(textChanged) {
				updateTextLayout();
			}

			if(layoutChanged) {
				layout();
			}

			Widget::update(delta);
		}

		void layout() override{
			textChanged = true;
			updateTextLayout();

			updateGlyphPosition();

			Widget::layout();
		}


		[[nodiscard]] bool isDynamic() const {
			return textSource != nullptr;
		}
	};
}