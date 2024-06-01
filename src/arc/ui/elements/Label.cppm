module;

export module UI.Label;

import ext.RuntimeException;
import UI.Align;
import UI.TextWidget;
import Font.GlyphArrangement;
import std;

export namespace UI {
	class Label : public TextWidget {
	protected:
		std::function<Font::TextString()> textSource{};

	public:
		[[nodiscard]] Label() = default;

		void setText(const Font::TextView text) {
			glyphLayout->lastText = text;
			textSource = nullptr;
			setTextUpdated();
		}

		void setText(Font::TextString&& text) {
			glyphLayout->lastText = std::move(text);
			textSource = nullptr;
			setTextUpdated();
		}

		void setText(const Font::TextView::const_pointer text) {
			glyphLayout->lastText = Font::TextView{text};
			textSource = nullptr;
			setTextUpdated();
		}

		void setText(Concepts::Invokable<Font::TextString()> auto&& charSource) {
			textSource = std::forward<decltype(charSource)>(charSource);
			glyphLayout->lastText = textSource();

			setTextUpdated();
		}

		void update(const Core::Tick delta) override {
			if(isDynamic()){
				if(Font::TextString cur = textSource(); cur != getText()){
					glyphLayout->lastText = std::move(cur);
					setTextUpdated();
				}

				layoutText(false);
			}else if(textChanged) {
				layoutText(false);
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