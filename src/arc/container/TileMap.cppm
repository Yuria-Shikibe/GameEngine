export module Container.TileMap;

import <utility>;
import <vector>;
import Concepts;
import RuntimeException;

export namespace Container {
	template <typename T>
	class TileMap
	// : std::mdspan<>
	{
		using sizeType = unsigned int;
		sizeType width{0};
		sizeType height{0};
		std::vector<T> items{};

	public:
		[[nodiscard]] TileMap(const sizeType width, const sizeType height)
			: width(width),
			height(height), items(width * height){
		}

		[[nodiscard]] TileMap(const sizeType width, const sizeType height, const T& object)
			: width(width),
			height(height), items(width * height, object){
		}

		[[nodiscard]] unsigned getWidth() const {
			return width;
		}

		[[nodiscard]] unsigned getHeight() const {
			return height;
		}

		[[nodiscard]] sizeType size() const {
			return width * height;
		}

		T& at(const sizeType x, sizeType y) {
#ifdef  _DEBUG
			if(x > width)throw ext::ArrayIndexOutOfBound{x, width};
			if(y > height)throw ext::ArrayIndexOutOfBound{y, height};
#endif

			return items.at(x + y * width);
		}

		[[nodiscard]] std::vector<T>& getItems() {
			return items;
		}

		void resize(const unsigned width, const unsigned height, const bool sustainLayout = false) {
			sizeType newSize = width * height;

			if(sustainLayout) {
				std::vector<T> newItems(newSize);

				for(int x = 0; x < this->width; ++x) {
					for(int y = 0; y < this->height; ++y) {
						newItems[x + y * width] = std::move(at(x, y));
					}
				}

				items = std::move(newItems);
			}else{
				items.resize(newSize);
			}

			this->width = width;
			this->height = height;
		}

		template <Concepts::Invokable<void(T&, sizeType, sizeType)> Func>
		void each(Func&& func) {
			for(int x = 0; x < this->width; ++x) {
				for(int y = 0; y < this->height; ++y) {
					func(at(x, y), x, y);
				}
			}
		}

		void clear() {
			items.clear();
		}
	};
}
