export module Container.Map2D;

import std;
import ext.Concepts;
import ext.RuntimeException;

export namespace Container {

	template <typename T>
	class Map2D
	// : std::mdspan<>
	{
		using SizeType = unsigned int;
		SizeType width{0};
		SizeType height{0};

		std::vector<T> items{};

		void boundCheck(const SizeType index) const noexcept(!DEBUG_CHECK){
#if DEBUG_CHECK
			if(index > size())throw ext::ArrayIndexOutOfBound{size(), width};
#endif
		}

		void boundCheck(const SizeType x, const SizeType y) const noexcept(!DEBUG_CHECK){
#if DEBUG_CHECK
			if(x > width)throw ext::ArrayIndexOutOfBound{x, width};
			if(y > height)throw ext::ArrayIndexOutOfBound{y, height};
#endif
		}

	public:
		[[nodiscard]] Map2D(const SizeType width, const SizeType height) noexcept
			: width(width),
			height(height), items(width * height){
		}

		[[nodiscard]] Map2D(const SizeType width, const SizeType height, const T& object) noexcept
			: width(width),
			height(height), items(width * height, object) {
		}

		[[nodiscard]] SizeType getWidth() const noexcept {
			return width;
		}

		[[nodiscard]] SizeType getHeight() const noexcept {
			return height;
		}

		[[nodiscard]] SizeType size() const {
			return width * height;
		}

		[[nodiscard]] T& at(const SizeType index){
			boundCheck(index);
			return items.at(index);
		}

		[[nodiscard]] const T& at(const SizeType index) const{
			boundCheck(index);
			return items.at(index);
		}

		[[nodiscard]] T& operator[](const SizeType index){return at(index);}
		const T& operator[](const SizeType index) const {return at(index);}

		[[nodiscard]] T& at(const SizeType x, const SizeType y) {
			boundCheck(x, y);

			return items.at(x + y * width);
		}

		[[nodiscard]] const T& at(const SizeType x, const SizeType y) const{
			boundCheck(x, y);

			return items.at(x + y * width);
		}

		[[nodiscard]] std::vector<T>& getItems() {
			return items;
		}

		void resize(const unsigned width, const unsigned height, const bool sustainLayout = false) requires std::is_default_constructible_v<T> {
			this->resize(T{}, width, height, sustainLayout);
		}

		void resize(const T& copyTarget, const unsigned width, const unsigned height, const bool sustainLayout = false) requires std::is_copy_constructible_v<T>{
			SizeType newSize = width * height;

			if(sustainLayout) {
				std::vector<T> newItems{newSize, copyTarget};

				for(SizeType x = 0; x < this->width; ++x) {
					for(SizeType y = 0; y < this->height; ++y) {
						newItems[x + y * width] = std::move(at(x, y));
					}
				}

				items = std::move(newItems);
			}else{
				items.resize(newSize, copyTarget);
			}

			this->width = width;
			this->height = height;
		}

		template <Concepts::Invokable<T(SizeType, SizeType)> Provider>
		void construct(const SizeType width, const SizeType height, Provider&& prov) requires std::is_move_constructible_v<T>{
			clear();

			this->width = width;
			this->height = height;

			items.reserve(width, height);

			for(SizeType x = 0; x < this->width; ++x) {
				for(SizeType y = 0; y < this->height; ++y) {
					items.push_back(prov(x, y));
				}
			}
		}

		template <Concepts::Invokable<void(T&, SizeType, SizeType)> Func>
		void each(Func&& func) {
			for(SizeType x = 0; x < this->width; ++x) {
				for(SizeType y = 0; y < this->height; ++y) {
					func(at(x, y), x, y);
				}
			}
		}

		template <Concepts::Invokable<void(T&)> Func>
		void each(Func&& func) {
			for(auto& element : items){
				func(element);
			}
		}

		void clear() {
			items.clear();
		}

		bool empty() const{
			return size() == 0;
		}
	};
}
