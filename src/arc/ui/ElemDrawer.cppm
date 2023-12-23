module;

export module UI.ElemDrawer;
import <memory>;
import Graphic.Color;
import GL.Texture.TextureRegion;
import GL.Texture.TextureNineRegion;

export namespace UI {
	class Elem;

	struct Drawable {
		virtual ~Drawable() = default;

		virtual void draw(float srcx, float srcy, float width, float height) const{

		}
	};

	struct TextureRegionRectDrawable : Drawable{
		GL::TextureRegion* rect{nullptr};

		[[nodiscard]] explicit TextureRegionRectDrawable(GL::TextureRegion* const rect)
			: rect(rect) {
		}

		void draw(float srcx, float srcy, float width, float height) const override;
	};

	struct TextureNineRegionDrawable : Drawable{
		GL::TextureNineRegion* rect{nullptr};

		[[nodiscard]] explicit TextureNineRegionDrawable(GL::TextureNineRegion* const rect)
			: rect(rect) {
		}

		void draw(float srcx, float srcy, float width, float height) const override;
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

		void draw(float srcx, float srcy, float width, float height) const;
	};

	struct UIStyle {
		DrawPair base{};
		DrawPair edge{};
		DrawPair inbound{};
		DrawPair pressed{};
		DrawPair disabled{};

		float margin_bottom{};
		float margin_top{};
		float margin_left{};
		float margin_right{};

		void drawElem(const Elem* elem) const;
	};

	struct ElemDrawer {
		const UIStyle* style{nullptr};

		virtual ~ElemDrawer() = default;

		[[nodiscard]] ElemDrawer() = default;

		[[nodiscard]] explicit ElemDrawer(const UIStyle& style)
			: style(&style) {
		}

		virtual void drawBackground(const UI::Elem* elem) const{
			if(style)style->drawElem(elem);
		}
	};

	struct EdgeDrawer final : ElemDrawer{
		void drawBackground(const UI::Elem* elem) const override;
	};

	struct EmptyDrawer final : ElemDrawer{
		void drawBackground(const UI::Elem* elem) const override {

		}
	};

	// std::unique_ptr<ElemDrawer> defDrawer{std::make_unique<EdgeDrawer>()};
	std::unique_ptr<ElemDrawer> defDrawer{std::make_unique<ElemDrawer>()};
	std::unique_ptr<ElemDrawer> emptyDrawer{std::make_unique<EmptyDrawer>()};
}
