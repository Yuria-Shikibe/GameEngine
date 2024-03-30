export module UI.ElemDrawer;
import std;
import Graphic.Color;
import GL.Texture.TextureRegion;
import GL.Texture.TextureNineRegion;
import Geom.Shape.Rect_Orthogonal;

export import UI.Align;

export namespace UI {
	class Elem;

	struct Drawable {
		virtual ~Drawable() = default;

		virtual void draw(Geom::OrthoRectFloat rect) const{

		}
	};

	struct TextureRegionRectDrawable : Drawable{
		GL::TextureRegion* texRegion{nullptr};

		[[nodiscard]] explicit TextureRegionRectDrawable(GL::TextureRegion* const rect)
			: texRegion(rect) {
		}

		void draw(Geom::OrthoRectFloat rect) const override;
	};

	struct TextureNineRegionDrawable : Drawable{
		GL::TextureNineRegion* texRegion{nullptr};

		[[nodiscard]] explicit TextureNineRegionDrawable(GL::TextureNineRegion* const rect)
			: texRegion(rect) {
		}

		void draw(Geom::OrthoRectFloat rect) const override;
	};

	struct DrawPair {
		const Drawable* region{};
		Graphic::Color color{};

		[[nodiscard]] DrawPair(const Drawable* region, const Graphic::Color& color)
			: region(region),
			color(color) {
		}

		[[nodiscard]] explicit DrawPair(const Drawable* region)
			: DrawPair(region, Graphic::Colors::WHITE) {
		}

		[[nodiscard]] DrawPair() = default;

		void draw(const UI::Elem* elem, float alphaScl, Geom::OrthoRectFloat rect) const;
	};

	struct UIStyle {
		DrawPair base{};
		DrawPair edge{};
		DrawPair inbound{};
		DrawPair pressed{};
		DrawPair disabled{};

		Align::Spacing margin{};

		void drawElem(const Elem* elem) const;
	};

	struct ElemDrawer {
		virtual ~ElemDrawer() = default;

		[[nodiscard]] ElemDrawer() = default;

		virtual void drawBackground(const UI::Elem* elem) const = 0;

		virtual void applyToElem(Elem* elem) {
		}
	};

	struct StyleDrawer : ElemDrawer{
		const UIStyle* style{nullptr};

		[[nodiscard]] StyleDrawer() = default;

		[[nodiscard]] explicit StyleDrawer(const UIStyle* style)
			: style(style) {
		}

		void drawBackground(const UI::Elem* elem) const override {
			if(style)style->drawElem(elem);
		}

		void applyToElem(Elem* elem) override;
	};

	struct EdgeDrawer final : ElemDrawer{
		void drawBackground(const UI::Elem* elem) const override;
	};

	struct EmptyDrawer final : ElemDrawer{
		void drawBackground(const UI::Elem* elem) const override {

		}

		void applyToElem(Elem* elem) override;
	};

	// std::unique_ptr<ElemDrawer> defDrawer{std::make_unique<EdgeDrawer>()};
	ElemDrawer* defDrawer{nullptr};
	EmptyDrawer emptyDrawer{};
}
