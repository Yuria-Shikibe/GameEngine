export module UI.Cursor;

export import UI.CursorType;
import Geom.Vector2D;
import Graphic.Draw;
import GL.Texture.TextureRegion;

import std;

export namespace UI{
	struct CursorThoroughSightDrawer : CursorAdditionalDrawer {
		float stroke{4};
		float margin{20};

		CursorThoroughSightDrawer(const float stroke = 4.0f, const float margin = 36.0f)
			: stroke(stroke),
			  margin(margin){
		}

		void operator()(const float x, const float y, const float w, const float h) override{
			using namespace Graphic;
			Draw::Overlay::Line::setLineStroke(h * 2);
			Draw::Overlay::Line::line(-1, y, x - w * margin, y);
			Draw::Overlay::Line::line(x + w * margin, y, 1, y);

			Draw::Overlay::Line::setLineStroke(w * 2);
			Draw::Overlay::Line::line(x, -1, x, y - margin * h);
			Draw::Overlay::Line::line(x, y + margin * h, x, 1);
		}
	};

	struct Cursor : CursorDrawabe {
		~Cursor() override = default;

		//TODO animation?
		const GL::TextureRegion* image{nullptr};
		Geom::Point2U offset{};
		Geom::Point2U size{50, 50};

		Cursor() = default;

		explicit Cursor(const GL::TextureRegion* image, const Geom::Point2U offset = Geom::ZERO_U, const Geom::Point2U size = {50, 50})
			: image(image),
			  offset(offset),
			  size(size){
		}

		void draw(const float x, const float y, const Geom::Vec2 screenSize, const float progress = 0.0f, const float scl = 1.0f) const override{
			const float norX = 2 / screenSize.x;
			const float norY = 2 / screenSize.y;
			const float width = size.x * scl * norX;
			const float height = size.y * scl * norY;
			const float drawX = x + offset.x * norX;
			const float drawY = y + offset.y * norY;
			if(drawer){
				drawer->operator()(drawX, drawY, norX, norY);
			}
			Graphic::Draw::Overlay::Fill::rectOrtho(
				*image, drawX - width / 2, drawY - height / 2,
				width, height
			);
		}

		void setImage(const GL::TextureRegion* const image){
			this->image = image;
			size.x = static_cast<unsigned int>(image->getWidth());
			size.y = static_cast<unsigned int>(image->getHeight());
		}
	};
}
