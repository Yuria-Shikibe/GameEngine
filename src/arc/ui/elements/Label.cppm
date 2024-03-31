module;

export module UI.Label;

import RuntimeException;
import UI.Align;
import UI.Elem;
import Font.GlyphArrangement;
import std;

export namespace UI {
	class Label : public Elem {
		using TextView = std::string_view;
	protected:
		std::shared_ptr<Font::GlyphLayout> glyphLayout{Font::obtainLayoutPtr()};

		Align::Mode textAlignMode{Align::Mode::top_left};
		std::function<std::string()> textSource{};

		bool textChanged = false;

	public:

		[[nodiscard]] TextView getLastText() const{
			return glyphLayout->lastText;
		};

		void setTextAlign(const Align::Mode align){
			textAlignMode = align;
			glyphLayout->setAlign(textAlignMode);
		}

		[[nodiscard]] Label() {
			setBorder(12.0f);
		}

		void textUpdated() {
			textChanged = true;
		}

		void updateTextLayout() {
			Font::glyphParser->parseWith(glyphLayout, getValidWidth(), textChanged);
			glyphLayout->setAlign(textAlignMode);
			textChanged = false;
		}

		void setLayoutDataPtr(const std::shared_ptr<Font::GlyphLayout>& layoutPtr) {
			glyphLayout = layoutPtr;
		}

		void setText(const TextView text) {
			glyphLayout->lastText = text;
			textSource = nullptr;
			textUpdated();
			changed();
		}

		void setText(const TextView::const_pointer text) {
			glyphLayout->lastText = std::string_view{text};
			textSource = nullptr;
			textUpdated();
			changed();
		}

		void setText(Concepts::Invokable<TextView()> auto&& charSource) {
			textSource = std::forward<decltype(charSource)>(charSource);
			glyphLayout->lastText = std::move(textSource());

			textUpdated();
			changed();
		}

		void drawContent() const override;

		void calAbsoluteSrc(Elem* parent) override {
			Elem::calAbsoluteSrc(parent);
			glyphLayout->offset.set(absoluteSrc.x, absoluteSrc.y + bound.getHeight()).add(Align::getOffsetOf(textAlignMode, border));
		}

		void update(float delta) override {
			if(textSource){
				std::string cur = std::move(textSource());

				if(cur != getLastText()){
					glyphLayout->lastText = std::move(cur);
					textChanged = true;
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