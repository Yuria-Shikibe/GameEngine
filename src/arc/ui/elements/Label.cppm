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
		bool dynamic{false};
		std::shared_ptr<Font::GlyphLayout> glyphLayout{Font::obtainLayoutPtr()};

		Align::Mode textAlignMode{Align::Mode::top_left};

		TextView text{""};
		std::function<std::string()> textSource{};

		bool textChanged = false;
		bool adoptHeight = false;

	public:
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
			this->text = text;
			glyphLayout->lastText = text;
			textSource = nullptr;
			textUpdated();
			changed();
		}

		void setText(const TextView::const_pointer text) {
			this->text = std::string_view{text};
			glyphLayout->lastText = text;
			textSource = nullptr;
			textUpdated();
			changed();
		}

		void setText(Concepts::Invokable<TextView()> auto&& charSource) {
			textSource = std::forward<decltype(charSource)>(charSource);
			glyphLayout->lastText = std::move(textSource());
			text = glyphLayout->lastText;

			textUpdated();
			changed();
		}

		TextView& getView() {
			return text;
		}

		void drawContent() const override;

		void calAbsoluteSrc(Elem* parent) override {
			Elem::calAbsoluteSrc(parent);
			glyphLayout->offset.set(absoluteSrc.x, absoluteSrc.y + bound.getHeight()).add(Align::getOffsetOf(textAlignMode, border));
		}

		void update(float delta) override {
			if(dynamic){
				if(textSource){
					glyphLayout->lastText = std::move(textSource());
					text = glyphLayout->lastText;
				}

				updateTextLayout();
			}else if(textChanged) {
				updateTextLayout();
				textChanged = false;
			}

			if(layoutChanged) {
				layout();
			}

			Elem::update(delta);
		}

		[[nodiscard]] bool isDynamic() const {
			return dynamic;
		}

		void setDynamic(const bool dynamic) {
			this->dynamic = dynamic;
		}
	};
}