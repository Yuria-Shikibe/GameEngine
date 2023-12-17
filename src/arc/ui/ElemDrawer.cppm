module;

export module UI.ElemDrawer;
import <memory>;
import Graphic.Color;

export namespace UI {
	struct ElemDrawer {
		virtual ~ElemDrawer() = default;

		bool hover{false};
		bool click{false};
		bool invalid{false};

		virtual void drawBackground(float x, float y, float width, float height, const Graphic::Color& color, float opacity) const = 0;
	};

	struct EdgeDrawer final : ElemDrawer{
		void drawBackground(float x, float y, float width, float height, const Graphic::Color& color, float opacity) const override;
	};

	struct EmptyDrawer final : ElemDrawer{
		void drawBackground(const float x, const float y, const float width, const float height, const Graphic::Color& color, const float opacity) const override {

		}
	};


	std::unique_ptr<EdgeDrawer> defDrawer{new EdgeDrawer};
	std::unique_ptr<EmptyDrawer> emptyDrawer{new EmptyDrawer};
}
