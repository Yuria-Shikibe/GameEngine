module;

export module Assets.Cursor;

import Geom.Vector2D;
import Graphic.Draw;
import GL.Texture.TextureRegionRect;

import <array>;

export namespace Assets{
	enum struct CursorType : size_t {
		general,
		clickable,
		//...
		count
	};

	struct Cursor {
		const GL::TextureRegionRect* image{nullptr};
		Geom::Point2U offset{};
		Geom::Point2U size{50, 50};

		Cursor() = default;

		explicit Cursor(const GL::TextureRegionRect* image, const Geom::Point2U& offset = Geom::ZERO_U, const Geom::Point2U& size = {50, 50})
			: image(image),
			  offset(offset),
			  size(size){
		}

		void draw(const float x, const float y, const Geom::Vec2 screenSize, const float scl = 1.0f) const{
			const float width = size.x * scl / screenSize.x;
			const float height = size.y * scl / screenSize.y;
			Graphic::Draw::rect(
				image, x + offset.x / screenSize.x - width / 2, y + offset.y / screenSize.y - height / 2,
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
	inline std::array<Cursor, static_cast<size_t>(CursorType::count)> allCursors{};

	Cursor& getCursor(CursorType type){
		return allCursors[static_cast<size_t>(type)];
	}
}
