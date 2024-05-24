export module UI.Drawer;

import std;
import Graphic.Color;
import GL.Texture.TextureRegion;
import GL.Texture.TextureNineRegion;
import Geom.Vector2D;
import UI.RegionDrawable;

export import UI.Align;

export namespace UI { //TODO bullshit virtual
	class Elem;
	class ScrollPane;
	class ProgressBar;
	class SliderBar;

	struct SliderBarDrawer{
		virtual ~SliderBarDrawer() = default;

		virtual void draw(const SliderBar* sliderBar) const;
	};

	struct ProgressBarDrawer{
		virtual ~ProgressBarDrawer() = default;

		virtual void draw(const ProgressBar* progressBar) const;
	};

	struct ScrollBarDrawer {
		float margin = 1.0f;
		Geom::Vec2 offsetScl{0.35f, 0.35f};
		Graphic::Color barColor{Graphic::Colors::GRAY};
		Graphic::Color pressedBarColor{Graphic::Colors::LIGHT_GRAY};
		GL::TextureNineRegion region{};
		virtual ~ScrollBarDrawer() = default;

		virtual void operator()(const ScrollPane* pane) const;
	};

	struct TextureRegionRectDrawable : RegionDrawable{
		GL::TextureRegion* texRegion{nullptr};

		[[nodiscard]] explicit TextureRegionRectDrawable(GL::TextureRegion* const rect)
			: texRegion(rect) {
		}

		void draw(Geom::OrthoRectFloat rect) const override;
	};

	struct DrawPair {
		const RegionDrawable* region{};
		Graphic::Color color{};

		[[nodiscard]] DrawPair(const RegionDrawable* region, const Graphic::Color& color)
			: region(region),
			color(color) {
		}

		[[nodiscard]] explicit DrawPair(const RegionDrawable* region)
			: DrawPair(region, Graphic::Colors::WHITE) {
		}

		[[nodiscard]] DrawPair() = default;

		void draw(const UI::Elem* elem, float alphaScl, Geom::OrthoRectFloat rect) const;
	};

	struct UIStyle {
		DrawPair baseMask{};
		DrawPair base{};
		DrawPair edge{};
		DrawPair inbound{};
		DrawPair pressed{};
		DrawPair disabled{};
		DrawPair activated{};

		Align::Spacing margin{};

		void drawElem(const Elem* elem) const;

		void drawBackground(const Elem* elem) const;

		void drawBackground(Geom::OrthoRectFloat rect) const;
	};

	struct WidgetDrawer {
		virtual ~WidgetDrawer() = default;

		[[nodiscard]] WidgetDrawer() = default;

		virtual void drawStyle(const UI::Elem* elem) const = 0;

		virtual void drawBackground(const UI::Elem* elem) const{}

		virtual void drawBackground(Geom::OrthoRectFloat rect){}

		virtual void applyToElem(Elem* elem) {
		}
	};

	struct StyleDrawer : WidgetDrawer{
		const UIStyle* style{nullptr};

		[[nodiscard]] StyleDrawer() = default;

		[[nodiscard]] explicit StyleDrawer(const UIStyle* style)
			: style(style) {
		}

		void drawBackground(const UI::Elem* elem) const override{
			if(style)style->drawBackground(elem);
		}

		void drawStyle(const UI::Elem* elem) const override {
			if(style)style->drawElem(elem);
		}

		void drawBackground(const Geom::OrthoRectFloat rect) override{
			if(style)style->drawBackground(rect);
		}

		void applyToElem(Elem* elem) override;
	};

	struct EdgeDrawer final : WidgetDrawer{
		void drawStyle(const UI::Elem* elem) const override;
	};

	struct EmptyDrawer final : WidgetDrawer{
		void drawStyle(const UI::Elem* elem) const override{}

		void applyToElem(Elem* elem) override;
	};

	// std::unique_ptr<ElemDrawer> defDrawer{std::make_unique<EdgeDrawer>()};
	WidgetDrawer* defDrawer{nullptr};
	EmptyDrawer emptyDrawer{};

	ScrollBarDrawer defScrollBarDrawer{};
	ProgressBarDrawer defProgressBarDrawer{};
	SliderBarDrawer defSlideBarDrawer{};
}
