module;

export module Assets.Cursor;

import Geom.Vector2D;
import Graphic.Draw;
import GL.Texture.TextureRegionRect;

import std;

export namespace Assets{
	using namespace Graphic;

	enum struct CursorType : size_t {
		regular,
		clickable,
		select,
		//...
		count
	};

	struct CursorAdditionalDrawer {
		virtual ~CursorAdditionalDrawer() = default;
		virtual void operator()(float x, float y, float w, float h) = 0;
	};

	struct CursorThoroughSightDrawer : CursorAdditionalDrawer {
		float stroke{4};
		float margin{20};

		CursorThoroughSightDrawer(const float stroke = 4.0f, const float margin = 36.0f)
			: stroke(stroke),
			  margin(margin){
		}

		void operator()(const float x, const float y, const float w, const float h) override{
			Draw::Line::setLineStroke(h * 2);
			Draw::Line::line(-1, y, x - w * margin, y);
			Draw::Line::line(x + w * margin, y, 1, y);

			Draw::Line::setLineStroke(w * 2);
			Draw::Line::line(x, -1, x, y - margin * h);
			Draw::Line::line(x, y + margin * h, x, 1);
		}
	};

	struct Cursor { //TODO animation?
		const GL::TextureRegionRect* image{nullptr};
		Geom::Point2U offset{};
		Geom::Point2U size{50, 50};

		std::unique_ptr<CursorAdditionalDrawer> drawer{nullptr};

		Cursor() = default;

		explicit Cursor(const GL::TextureRegionRect* image, const Geom::Point2U& offset = Geom::ZERO_U, const Geom::Point2U& size = {50, 50})
			: image(image),
			  offset(offset),
			  size(size){
		}

		virtual void draw(const float x, const float y, const Geom::Vec2 screenSize, const float progress = 0.0f, const float scl = 1.0f) const{
			const float norX = 2 / screenSize.x;
			const float norY = 2 / screenSize.y;
			const float width = size.x * scl * norX;
			const float height = size.y * scl * norY;
			const float drawX = x + offset.x * norX;
			const float drawY = y + offset.y * norY;
			if(drawer){
				drawer->operator()(drawX, drawY, norX, norY);
			}
			Draw::rect(
				image, drawX - width / 2, drawY - height / 2,
				width, height
			);
		}

		void setImage(const GL::TextureRegionRect* const image){
			this->image = image;
			size.x = image->getWidth();
			size.y = image->getHeight();
		}
	};

	//TODO is unordered_map better for extension?
	inline std::array<std::unique_ptr<Cursor>, static_cast<size_t>(CursorType::count)> allCursors{};

	Cursor& getCursor(CursorType type){
		return *allCursors[static_cast<size_t>(type)];
	}

	std::unique_ptr<Cursor>& getCursorRaw(CursorType type){
		return allCursors[static_cast<size_t>(type)];
	}
}
