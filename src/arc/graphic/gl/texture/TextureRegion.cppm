export module GL.Texture.TextureRegion;

export import GL.Texture.Texture2D;

import <utility>;

export namespace GL{
	class TextureRegion{
	protected:
		/**
		 * \brief Does Not Hold The Ownership Of Source Texture, just borrow it!
		 * */
		const Texture2D* data = nullptr;
	public:
		virtual ~TextureRegion() = default;

		TextureRegion() = default;

		[[nodiscard]] explicit TextureRegion(const Texture2D* const data)
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

		[[nodiscard]] const Texture2D* getData() const {
			return data;
		}

		void setData(const Texture2D* const texture2D){
			data = texture2D;
		}

		void setData(const Texture2D& texture2D){
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
