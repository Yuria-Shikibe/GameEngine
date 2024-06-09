export module GL.Texture.TextureRegion;

import ext.Concepts;
import Geom.Rect_Orthogonal;
import Geom.Vector2D;
export import GL.Texture.Texture2D;

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
	class TextureRegion{
	public:
		const Texture* data = nullptr;

		Geom::Vec2
			v00, v10, v01, v11;

		constexpr TextureRegion() = default;


		[[nodiscard]] constexpr TextureRegion(const float u0, const float v0, const float u1, const float v1) // NOLINT(*-easily-swappable-parameters)
			: v00{u0, v0},
			  v10(u1, v0),
			  v01(u0, v1),
			  v11(u1, v1) {
		}

		[[nodiscard]] constexpr TextureRegion(const GL::Texture2D* const data, const float u0, const float v0,
		                                          const float u1, const float v1)
			: data(data),
			  v00{u0, v0},
			  v10(u1, v0),
			  v01(u0, v1),
			  v11(u1, v1){}

		[[nodiscard]] explicit TextureRegion(const Texture2D* const data) : TextureRegion(data, 0.0, 0.0, 1.0f, 1.0f) {

		}

		[[nodiscard]] TextureRegion(const Texture* data, Geom::Vec2 v00, Geom::Vec2 v10, Geom::Vec2 v01, Geom::Vec2 v11)
			: data{data},
			  v00{v00},
			  v10{v10},
			  v01{v01},
			  v11{v11}{}

		void resize(const float u0, const float v0, const float u1, const float v1) {
			v00.set(u0, v0);
			v10.set(u1, v0);
			v01.set(u0, v1);
			v11.set(u1, v1);
		}

		void flipY() {
			std::swap(v00, v01);
			std::swap(v10, v11);
		}

		void flipX() {
			std::swap(v00, v10);
			std::swap(v01, v11);
		}

		//Below functions work properly only on rect regions

		constexpr void copyPositionDataFrom(const TextureRegion& other) noexcept{
			v00 = other.v00;
			v10 = other.v10;
			v01 = other.v01;
			v11 = other.v11;
		}

		[[nodiscard]] float getSrcX() const {
			return v00.x * data->getWidth();
		}

		[[nodiscard]] float getSrcY() const {
			return v00.y * data->getHeight();
		}

		[[nodiscard]] float getWidth() const {
			return Math::abs(v00.x - v10.x) * static_cast<float>(data->getWidth());
		}

		[[nodiscard]] float getHeight() const {
			return Math::abs(v00.y - v01.y) * static_cast<float>(data->getHeight());
		}

		Geom::Vec2 getSize() const{
			return {getWidth(), getHeight()};
		}

		void shrinkEdgeX(const float marginX) {
			Geom::OrthoRectFloat newBound{v00.x * data->getWidth(), v00.y * data->getHeight(), getWidth(), getHeight()};
			newBound.move(marginX, 0).addSize(-marginX * 2.0f, 0);
			fetchIntoCurrent(newBound);
		}

		void shrinkEdgeY(const float marginY) {
			Geom::OrthoRectFloat newBound{v00.x * data->getWidth(), v00.y * data->getHeight(), getWidth(), getHeight()};
			newBound.move(0, marginY).addSize(0, -marginY * 2.0f);
			fetchIntoCurrent(newBound);
		}

		void shrinkEdge(const float marginX, const float marginY) {
			Geom::OrthoRectFloat newBound{v00.x * data->getWidth(), v00.y * data->getHeight(), getWidth(), getHeight()};
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

			const float u0 = srcx + static_cast<float>(internal.getSrcX()) / static_cast<float>(external.getWidth());
			const float v0 = srcy + static_cast<float>(internal.getSrcY()) / static_cast<float>(external.getHeight());

			const float u1 = srcx + static_cast<float>(internal.getEndX()) / static_cast<float>(external.getWidth());
			const float v1 = srcy + static_cast<float>(internal.getEndY()) / static_cast<float>(external.getHeight());

			resize(u0, v0, u1, v1);
		}

		template<Concepts::Number N0>
		void fetchIntoCurrent(const Geom::Rect_Orthogonal<N0>& internal) {
			const float u0 = static_cast<float>(internal.getSrcX()) / static_cast<float>(data->getWidth());
			const float v0 = static_cast<float>(internal.getSrcY()) / static_cast<float>(data->getHeight());

			const float u1 = static_cast<float>(internal.getEndX()) / static_cast<float>(data->getWidth());
			const float v1 = static_cast<float>(internal.getEndY()) / static_cast<float>(data->getHeight());

			resize(u0, v0, u1, v1);
		}

		template<Concepts::Number N0, Concepts::Number N1>
		void fetchInto(const Geom::Rect_Orthogonal<N0>& internal, const N1 width, const N1 height) {
			const float u0 = static_cast<float>(internal.getSrcX()) / static_cast<float>(width);
			const float v0 = static_cast<float>(internal.getSrcY()) / static_cast<float>(height);

			const float u1 = static_cast<float>(internal.getEndX()) / static_cast<float>(width);
			const float v1 = static_cast<float>(internal.getEndY()) / static_cast<float>(height);

			resize(u0, v0, u1, v1);
		}

		template<Concepts::Number N0, Concepts::Number N1>
		void fetchInto(const N0 srcX, const N0 srcY, const N0 width, const N0 height, const Geom::Rect_Orthogonal<N1>& external) {
			float srcx = 0.0f;
			float srcy = 0.0f;
			if(data != nullptr) {
				srcx = static_cast<float>(external.getSrcX()) / static_cast<float>(data->getWidth());
				srcy = static_cast<float>(external.getSrcY()) / static_cast<float>(data->getHeight());
			}

			const float u0 = srcx + static_cast<float>(srcX) / static_cast<float>(external.getWidth());
			const float v0 = srcy + static_cast<float>(srcY) / static_cast<float>(external.getHeight());

			const float u1 = srcx + static_cast<float>(srcX + width) / static_cast<float>(external.getWidth());
			const float v1 = srcy + static_cast<float>(srcY + height) / static_cast<float>(external.getHeight());

			resize(u0, v0, u1, v1);
		}

		friend bool operator==(const TextureRegion& lhs, const TextureRegion& rhs){
			return lhs.data == rhs.data
				&& lhs.v00 == rhs.v00
				&& lhs.v10 == rhs.v10
				&& lhs.v01 == rhs.v01
				&& lhs.v11 == rhs.v11;
		}

		friend bool operator!=(const TextureRegion& lhs, const TextureRegion& rhs){ return !(lhs == rhs); }
	};
}



