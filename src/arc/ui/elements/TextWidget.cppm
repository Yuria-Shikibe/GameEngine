//
// Created by Matrix on 2024/5/23.
//

export module UI.TextWidget;

export import UI.Elem;
import std;
import Font.GlyphArrangement;

export namespace UI{
	class TextWidget : public UI::Elem{
	protected:
		std::shared_ptr<Font::GlyphLayout> glyphLayout{Font::obtainLayoutPtr()};
		const Font::GlyphParser* defParser = Font::defGlyphParser.get();

		bool textChanged = false;

		bool usingGlyphHeight = false;
		bool usingGlyphWidth = false;
		Align::Mode textAlignMode{Align::Mode::top_left};

		void updateGlyphPosition() const{
			const Rect& drawBound = glyphLayout->getDrawBound();
			const auto offset = Align::getOffsetOf(textAlignMode, drawBound.getSize(), getValidBound());
			glyphLayout->offset.set(offset).add(absoluteSrc);
		}

		void setTextUpdated() {
			textChanged = true;
		}

		void layoutText(const bool forceUpdate = false){
			float expectedWidth = getValidWidth();
			if(Math::zero(expectedWidth)){
				expectedWidth = std::numeric_limits<float>::max();
			}

			defParser->parseWith(glyphLayout, usingGlyphWidth ? std::numeric_limits<float>::max() : expectedWidth, textChanged || forceUpdate);
			glyphLayout->setAlign(Align::Mode::bottom_left);
			textChanged = false;

			if(usingGlyphHeight && !fillParentY){
				setHeight(border.getHeight() + glyphLayout->getDrawBound().getHeight());
			}

			if(usingGlyphWidth && !fillParentX){
				setWidth(border.getWidth() + glyphLayout->getDrawBound().getWidth());
			}
		}
	public:
		void calAbsoluteSrc(Elem* parent) noexcept override {
			Elem::calAbsoluteSrc(parent);
			updateGlyphPosition();
		}

		void setTextAlign(const Align::Mode align){
			textAlignMode = align;
			updateGlyphPosition();
		}

		[[nodiscard]] Align::Mode getTextAlignMode() const noexcept{ return textAlignMode; }

		void layout() override{
			setTextUpdated();
			layoutText();
			updateGlyphPosition();

			Elem::layout();
		}

		void setTextScl(const float scl = 1.0f) const noexcept{
			glyphLayout->setSCale(scl);
		}

		constexpr void setWrap(const bool wrapX = true, const bool wrapY = true) noexcept{
			usingGlyphHeight = wrapY;
			usingGlyphWidth = wrapX;
		}

		[[nodiscard]] std::shared_ptr<Font::GlyphLayout>& getGlyphLayout() noexcept{
			return glyphLayout;
		}

		void setLayoutDataPtr(const std::shared_ptr<Font::GlyphLayout>& layoutPtr) noexcept{
			glyphLayout = layoutPtr;
		}

		[[nodiscard]] const Font::TextString& getText() const & {
			return glyphLayout->lastText;
		}

		[[nodiscard]] Font::TextString getText() const && {
			return std::move(glyphLayout->lastText);
		}

		[[nodiscard]] Font::TextString& getTextRef() const{
			return glyphLayout->lastText;
		}

		[[nodiscard]] Font::TextView getTextView() const {
			return glyphLayout->lastText;
		}

		void drawContent() const override;
	};
}
