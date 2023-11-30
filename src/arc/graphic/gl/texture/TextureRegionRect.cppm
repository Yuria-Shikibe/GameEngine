export module GL.Texture.TextureRegionRect;

import Concepts;
import Geom.Shape.Rect_Orthogonal;
import GL.Texture.Texture2D;
import GL.Texture.TextureRegion;
import <tuple>;

export namespace GL{
	class TextureRegionRect final : virtual public TextureRegion
	{
	public:
		~TextureRegionRect() override = default;

		float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;

		TextureRegionRect() = default;

		/*	| y
		 *  |   01(u0, v1)         11(u1, v1)
		 *  |
		 *  |             --IMAGE--
		 *  |
		 *  |   00(u0, v0)         10(u1, v0)
		 *  + ------------------------------> x (Normally)
		 * */


		[[nodiscard]] TextureRegionRect(const float u0, const float v0, const float u1, const float v1)
			: u0(u0),
			  v0(v0),
			  u1(u1),
			  v1(v1) {
		}

		[[nodiscard]] TextureRegionRect(const Texture2D* const data, const float u0, const float v0, const float u1, const float v1)
			: TextureRegion(data),
			  u0(u0),
			  v0(v0),
			  u1(u1),
			  v1(v1) {
		}

		[[nodiscard]] explicit TextureRegionRect(const Texture2D* const data) : TextureRegionRect(data, 0.0, 0.0, 1.0f, 1.0f) {

		}

		void resize(const float _u0, const float _v0, const float _u1, const float _v1) {
			u0 = _u0;
			v0 = _v0;
			u1 = _u1;
			v1 = _v1;
		}

		[[nodiscard]] const Texture2D& texture() const{
			return *data;
		}

		void setTexture(const Texture2D& texture2D){
			data = &texture2D;
		}

		[[nodiscard]] float u00() const override{return u0;}
		[[nodiscard]] float u01() const override{return u0;}
		[[nodiscard]] float u11() const override{return u1;}
		[[nodiscard]] float u10() const override{return u1;}

		[[nodiscard]] float v00() const override{return v0;}
		[[nodiscard]] float v01() const override{return v1;}
		[[nodiscard]] float v11() const override{return v1;}
		[[nodiscard]] float v10() const override{return v0;}

		TextureRegionRect(const TextureRegionRect& other) = default;

		TextureRegionRect(TextureRegionRect&& other) noexcept
			: TextureRegion(std::move(other)),
			  u0(other.u0),
			  v0(other.v0),
			  u1(other.u1),
			  v1(other.v1) {
		}

		TextureRegionRect& operator=(const TextureRegionRect& other) {
			if(this == &other) return *this;
			TextureRegion::operator =(other);
			u0 = other.u0;
			v0 = other.v0;
			u1 = other.u1;
			v1 = other.v1;
			return *this;
		}

		TextureRegionRect& operator=(TextureRegionRect&& other) noexcept {
			if(this == &other) return *this;
			TextureRegion::operator =(std::move(other));
			u0 = other.u0;
			v0 = other.v0;
			u1 = other.u1;
			v1 = other.v1;
			return *this;
		}

		void flipY() {
			std::swap(v0, v1);
		}

		void flipX() {
			std::swap(u0, u1);
		}

		template<Concepts::Number N0, Concepts::Number N1>
		void fetchInto(const Geom::Shape::Rect_Orthogonal<N0>& internal, const Geom::Shape::Rect_Orthogonal<N1>& external) {
			u0 = static_cast<float>(internal.getSrcX()) / static_cast<float>(external.getWidth());
			v0 = static_cast<float>(internal.getSrcY()) / static_cast<float>(external.getHeight());

			u1 = static_cast<float>(internal.getEndX()) / static_cast<float>(external.getWidth());
			v1 = static_cast<float>(internal.getEndY()) / static_cast<float>(external.getHeight());
		}

		friend bool operator==(const TextureRegionRect& lhs, const TextureRegionRect& rhs) {
			return lhs.data == rhs.data
			       && lhs.u0 == rhs.u0
			       && lhs.v0 == rhs.v0
			       && lhs.u1 == rhs.u1
			       && lhs.v1 == rhs.v1;
		}

		friend bool operator!=(const TextureRegionRect& lhs, const TextureRegionRect& rhs) {
			return !(lhs == rhs);
		}

		friend void swap(TextureRegionRect& lhs, TextureRegionRect& rhs) noexcept {
			using std::swap;
			swap(lhs.data, rhs.data);
			swap(lhs.u0, rhs.u0);
			swap(lhs.v0, rhs.v0);
			swap(lhs.u1, rhs.u1);
			swap(lhs.v1, rhs.v1);
		}
	};
}



