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
		using TextView = std::string_view;
	protected:
		bool dynamic{false};
		std::shared_ptr<Font::GlyphLayout> glyphLayout{Font::obtainLayoutPtr()};
		Align::Mode textAlignMode{Align::Mode::top_left};

		//TODO Support characters apart from ASCII
		TextView text = "";
		std::shared_ptr<TextView::value_type> dataSource{nullptr};

		bool textChanged = false;

	public:
		void textUpdated() {
			textChanged = true;
		}

		void updateTextLayout() const {
			glyphLayout->maxWidth = bound.getWidth();
			Font::glyphParser->parse(glyphLayout, text);
			glyphLayout->setAlign(textAlignMode);
		}

		void setLayoutDataPtr(const std::shared_ptr<Font::GlyphLayout>& layoutPtr) {
			glyphLayout = layoutPtr;
		}

		void setText(const TextView text) {
			this->text = text;
			textUpdated();
			changed();
		}

		void setText(const TextView::const_pointer text) {
			this->text = std::string_view{text};
			textUpdated();
			changed();
		}

		void setText(std::shared_ptr<TextView::value_type> text) {
			dataSource = std::move(text);
			this->text = std::string_view{dataSource.get()};
			textUpdated();
			changed();
		}

		TextView& getView() {
			return text;
		}

		void draw() const override {
			drawBackground();

			glyphLayout->offset.set(absoluteSrc.x, absoluteSrc.y + bound.getHeight());
			glyphLayout->render();
		}

		void layout() override {

			Elem::layout();
		}

		void update(float delta) override {
			if(dynamic)updateTextLayout();
			else if(textChanged) {
				updateTextLayout();
				textChanged = false;
			}

			if(layoutChanged) {
				layout();
			}
		}

		[[nodiscard]] bool isDynamic() const {
			return dynamic;
		}

		void setDynamic(const bool dynamic) {
			this->dynamic = dynamic;
		}

		void childrenCheck(const Elem* ptr) override {
			throw ext::IllegalArguments{"Labels shouldn't have children!"};
		}
	};
}
