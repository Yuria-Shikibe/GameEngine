export module GL.Texture.TextureRegion;

export import GL.Texture;

import std;

export namespace GL{
	class TextureRegion{
	protected:
		/**
		 * \brief Does Not Hold The Ownership Of Source Texture, just borrow it!
		 * */
		const Texture* data = nullptr;
	public:
		virtual ~TextureRegion() = default;

		constexpr TextureRegion() = default;

		[[nodiscard]] explicit constexpr TextureRegion(const Texture* const data)
			: data(data) {
		}

		friend bool operator==(const TextureRegion& lhs, const TextureRegion& rhs) {
			return lhs.data == rhs.data;
		}

		friend bool operator!=(const TextureRegion& lhs, const TextureRegion& rhs) {
			return !(lhs == rhs);
		}

		friend void swap(TextureRegion& lhs, TextureRegion& rhs) noexcept {
			using std::swap;
			swap(lhs.data, rhs.data);
		}

		[[nodiscard]] const Texture* getData() const {
			return data;
		}

		void setData(const Texture* const texture2D){
			data = texture2D;
		}

		void setData(const Texture& texture2D){
			data = &texture2D;
		}

		[[nodiscard]] virtual float u00() const = 0;
		[[nodiscard]] virtual float u01() const = 0;
		[[nodiscard]] virtual float u11() const = 0;
		[[nodiscard]] virtual float u10() const = 0;

		[[nodiscard]] virtual float v00() const = 0;
		[[nodiscard]] virtual float v01() const = 0;
		[[nodiscard]] virtual float v11() const = 0;
		[[nodiscard]] virtual float v10() const = 0;
	};
}
