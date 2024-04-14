export module GL.Texture.TextureRegionRect;

import Concepts;
import Geom.Rect_Orthogonal;
export import GL.Texture.Texture2D;
export import GL.Texture.TextureRegion;

import std;
import Math;

export namespace GL{
	/**
	 * \brief
	 * @code
	 * y
	 * ^
	 * |
	 * |   01(u0, v1)         11(u1, v1)
	 * |
	 * |             --IMAGE--
	 * |
	 * |   00(u0, v0)         10(u1, v0)
	 * +-------------------------------> x (Normally)
	 * @endcode
	 */
	class TextureRegionRect final : public TextureRegion{
	public:
		~TextureRegionRect() override = default;

		float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;

		constexpr TextureRegionRect() = default;


		[[nodiscard]] constexpr TextureRegionRect(const float u0, const float v0, const float u1, const float v1) // NOLINT(*-easily-swappable-parameters)
			: u0(u0),
			  v0(v0),
			  u1(u1),
			  v1(v1) {
		}

		[[nodiscard]] constexpr TextureRegionRect(const GL::Texture2D* const data, const float u0, const float v0, const float u1, const float v1)
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

		[[nodiscard]] float u00() const override{return u0;}
		[[nodiscard]] float u01() const override{return u0;}
		[[nodiscard]] float u11() const override{return u1;}
		[[nodiscard]] float u10() const override{return u1;}

		[[nodiscard]] float v00() const override{return v0;}
		[[nodiscard]] float v01() const override{return v1;}
		[[nodiscard]] float v11() const override{return v1;}
		[[nodiscard]] float v10() const override{return v0;}

		void flipY() {
			std::swap(v0, v1);
		}

		void flipX() {
			std::swap(u0, u1);
		}

		[[nodiscard]] float getWidth() const {
			return Math::abs(u1 - u0) * static_cast<float>(data->getWidth());
		}

		[[nodiscard]] float getHeight() const {
			return Math::abs(v1 - v0) * static_cast<float>(data->getHeight());
		}

		void shrinkEdgeX(const float marginX) {
			Geom::OrthoRectFloat newBound{u0 * data->getWidth(), v0 * data->getHeight(), getWidth(), getHeight()};
			newBound.move(marginX, 0).addSize(-marginX * 2.0f, 0);
			fetchIntoCurrent(newBound);
		}

		void shrinkEdgeY(const float marginY) {
			Geom::OrthoRectFloat newBound{u0 * data->getWidth(), v0 * data->getHeight(), getWidth(), getHeight()};
			newBound.move(0, marginY).addSize(0, -marginY * 2.0f);
			fetchIntoCurrent(newBound);
		}

		void shrinkEdge(const float marginX, const float marginY) {
			Geom::OrthoRectFloat newBound{u0 * data->getWidth(), v0 * data->getHeight(), getWidth(), getHeight()};
			newBound.move(marginX, marginY).addSize(-marginX * 2.0f, -marginY * 2.0f);
			fetchIntoCurrent(newBound);
		}

		void shrinkEdge(const float margin) {
			shrinkEdge(margin, margin);
		}

		template<Concepts::Number N0, Concepts::Number N1>
		void fetchInto(const Geom::Rect_Orthogonal<N0>& internal, const Geom::Rect_Orthogonal<N1>& external) {
			float srcx = 0.0f;
			float srcy = 0.0f;
			if(data != nullptr) {
				srcx = static_cast<float>(external.getSrcX()) / static_cast<float>(data->getWidth());
				srcy = static_cast<float>(external.getSrcY()) / static_cast<float>(data->getHeight());
			}

			u0 = srcx + static_cast<float>(internal.getSrcX()) / static_cast<float>(external.getWidth());
			v0 = srcy + static_cast<float>(internal.getSrcY()) / static_cast<float>(external.getHeight());

			u1 = srcx + static_cast<float>(internal.getEndX()) / static_cast<float>(external.getWidth());
			v1 = srcy + static_cast<float>(internal.getEndY()) / static_cast<float>(external.getHeight());
		}

		template<Concepts::Number N0>
		void fetchIntoCurrent(const Geom::Rect_Orthogonal<N0>& internal) {
			u0 = static_cast<float>(internal.getSrcX()) / static_cast<float>(getData()->getWidth());
			v0 = static_cast<float>(internal.getSrcY()) / static_cast<float>(getData()->getHeight());

			u1 = static_cast<float>(internal.getEndX()) / static_cast<float>(getData()->getWidth());
			v1 = static_cast<float>(internal.getEndY()) / static_cast<float>(getData()->getHeight());
		}

		template<Concepts::Number N0, Concepts::Number N1>
		void fetchInto(const Geom::Rect_Orthogonal<N0>& internal, const N1 width, const N1 height) {
			u0 = static_cast<float>(internal.getSrcX()) / static_cast<float>(width);
			v0 = static_cast<float>(internal.getSrcY()) / static_cast<float>(height);

			u1 = static_cast<float>(internal.getEndX()) / static_cast<float>(width);
			v1 = static_cast<float>(internal.getEndY()) / static_cast<float>(height);
		}

		template<Concepts::Number N0, Concepts::Number N1>
		void fetchInto(const N0 srcX, const N0 srcY, const N0 width, const N0 height, const Geom::Rect_Orthogonal<N1>& external) {
			float srcx = 0.0f;
			float srcy = 0.0f;
			if(data != nullptr) {
				srcx = static_cast<float>(external.getSrcX()) / static_cast<float>(data->getWidth());
				srcy = static_cast<float>(external.getSrcY()) / static_cast<float>(data->getHeight());
			}

			u0 = srcx + static_cast<float>(srcX) / static_cast<float>(external.getWidth());
			v0 = srcy + static_cast<float>(srcY) / static_cast<float>(external.getHeight());

			u1 = srcx + static_cast<float>(srcX + width) / static_cast<float>(external.getWidth());
			v1 = srcy + static_cast<float>(srcY + height) / static_cast<float>(external.getHeight());
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



