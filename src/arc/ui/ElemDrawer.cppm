module;

export module UI.ElemDrawer;
import <memory>;
import Graphic.Color;

export namespace UI {
	class Elem;

	struct ElemDrawer {
		virtual ~ElemDrawer() = default;

		virtual void drawBackground(const UI::Elem* elem) const = 0;
	};

	struct EdgeDrawer final : ElemDrawer{
		void drawBackground(const UI::Elem* elem) const override;
	};

	struct EmptyDrawer final : ElemDrawer{
		void drawBackground(const UI::Elem* elem) const override {

		}
	};

	std::unique_ptr defDrawer{std::make_unique<EdgeDrawer>()};
}
