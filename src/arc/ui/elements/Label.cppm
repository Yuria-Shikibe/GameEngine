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
		TextView text{" "};
		std::shared_ptr<TextView::value_type> dataSource{nullptr};

		bool textChanged = false;
		bool adoptHeight = false;

	public:
		[[nodiscard]] Label() {
			setMargin(12.0f);
			setText(" ");
		}

		void textUpdated() {
			textChanged = true;
		}

		void updateTextLayout() const {
			Font::glyphParser->parse(glyphLayout, text, bound.getWidth() - marginWidth());
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

		void drawContent() const override;

		void calAbsolute(Elem* parent) override {
			Elem::calAbsolute(parent);
			glyphLayout->offset.set(absoluteSrc.x, absoluteSrc.y + bound.getHeight()).add(Align::motionOf(textAlignMode, margin_bottomLeft, margin_topRight));
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