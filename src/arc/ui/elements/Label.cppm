module;

export module UI.Label;

import RuntimeException;
import Align;
import UI.Elem;
import GlyphArrangement;
import <memory>;
import <string>;

export namespace UI {
	class Label : public Elem {
	protected:
		bool dynamic{false};
		std::shared_ptr<Font::GlyphLayout> glyphLayout{Font::obtainLayoutPtr()};
		Align::Mode textAlignMode{Align::Mode::top_left};

		std::string_view text = "";

	public:
		void updateTextLayout() const {
			glyphLayout->maxWidth = bound.getWidth();
			Font::glyphParser->parse(glyphLayout, text);
			glyphLayout->setAlign(textAlignMode);
		}

		void setLayoutDataPtr(const std::shared_ptr<Font::GlyphLayout>& layoutPtr) {
			glyphLayout = layoutPtr;
		}

		void setText(const std::string_view text) {
			this->text = text;
			changed();
		}

		void draw() const override {
			drawBackground();

			glyphLayout->offset.set(absoluteSrc.x, absoluteSrc.y + bound.getHeight());
			glyphLayout->render();
		}

		void layout() override {
			updateTextLayout();
			Elem::layout();
		}

		void update(float delta) override {
			if(layoutChanged || dynamic) {
				layout();
			}
		}

		void childrenCheck(const Elem* ptr) override {
			throw ext::IllegalArguments{"Labels shouldn't have children!"};
		}
	};
}
