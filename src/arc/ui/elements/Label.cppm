module;

export module UI.Label;

import RuntimeException;
import UI.Align;
import UI.Elem;
import Font.GlyphArrangement;
import std;

export namespace UI {
	class Label : public Elem {
	protected:
		std::shared_ptr<Font::GlyphLayout> glyphLayout{Font::obtainLayoutPtr()};

		Align::Mode textAlignMode{Align::Mode::top_left};
		std::function<Font::TextString()> textSource{};

		bool textChanged = false;

		void textUpdated() {
			textChanged = true;
		}

		void updateTextLayout() {
			Font::defGlyphParser->parseWith(glyphLayout, usingGlyphWidth ? std::numeric_limits<float>::max() : getValidWidth(), textChanged);
			glyphLayout->setAlign(textAlignMode);
			textChanged = false;
			if(usingGlyphHeight){
				setHeight(border.getHeight() + glyphLayout->bound.getHeight());
			}

			if(usingGlyphWidth){
				setWidth(border.getWidth() + glyphLayout->bound.getHeight());
			}
		}

	public:
		bool usingGlyphHeight = false;
		bool usingGlyphWidth = false;

		[[nodiscard]] Font::TextView getLastText() const{
			return glyphLayout->lastText;
		}

		void setTextAlign(const Align::Mode align){
			textAlignMode = align;
			glyphLayout->setAlign(textAlignMode);
		}

		[[nodiscard]] Label() {
			setBorder(12.0f);
		}

		void setLayoutDataPtr(const std::shared_ptr<Font::GlyphLayout>& layoutPtr) {
			glyphLayout = layoutPtr;
		}

		void setText(const Font::TextView text) {
			glyphLayout->lastText = text;
			textSource = nullptr;
			textUpdated();
		}

		void setText(const Font::TextView::const_pointer text) {
			glyphLayout->lastText = Font::TextView{text};
			textSource = nullptr;
			textUpdated();
		}

		void setText(Concepts::Invokable<Font::TextView()> auto&& charSource) {
			textSource = std::forward<decltype(charSource)>(charSource);
			glyphLayout->lastText = std::move(textSource());

			textUpdated();
		}

		void drawContent() const override;

		void calAbsoluteSrc(Elem* parent) override {
			Elem::calAbsoluteSrc(parent);
			glyphLayout->offset.set(absoluteSrc.x, absoluteSrc.y + bound.getHeight()).add(Align::getOffsetOf(textAlignMode, border));
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

			Elem::update(delta);
		}

		[[nodiscard]] bool isDynamic() const {
			return textSource != nullptr;
		}
	};
}