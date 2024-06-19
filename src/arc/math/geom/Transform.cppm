//
// Created by Matrix on 2024/3/21.
//

export module Geom.Transform;

export import Geom.Vector2D;

import ext.Concepts;
import std;

export namespace Geom{
	struct Transform {
		Vec2 vec{};
		float rot{};

		constexpr void setZero(){
			vec.setZero();
			rot = 0.0f;
		}

		template <float NaN = std::numeric_limits<float>::signaling_NaN()>
		constexpr void setNan(){
			vec.set(NaN, NaN);
			rot = NaN;
		}

		constexpr Transform& applyInv(const Transform& transform) noexcept{
			vec.sub(transform.vec).rotate(-transform.rot);
			rot -= transform.rot;

			return *this;
		}

		constexpr Transform& apply(const Transform& transform) noexcept{
			vec.rotate(transform.rot).add(transform.vec);
			rot += transform.rot;

			return *this;
		}

		constexpr Transform& operator|=(const Transform& parentRef) noexcept{
			return apply(parentRef);
		}

		constexpr Transform& operator+=(const Transform& other) noexcept{
			vec += other.vec;
			rot += other.rot;

			return *this;
		}

		constexpr Transform& operator-=(const Transform& other) noexcept{
			vec -= other.vec;
			rot -= other.rot;

			return *this;
		}

		constexpr Transform& operator*=(const float scl) noexcept{
			vec *= scl;
			rot *= scl;

			return *this;
		}

		[[nodiscard]] constexpr friend Transform operator*(Transform self, const float scl) noexcept{
			return self *= scl;
		}

		[[nodiscard]] constexpr friend Transform operator+(Transform self, const Transform& other) noexcept{
			return self += other;
		}

		[[nodiscard]] constexpr friend Transform operator-(Transform self, const Transform& other) noexcept{
			return self -= other;
		}

		[[nodiscard]] constexpr friend Transform operator-(Transform self) noexcept{
			self.vec.reverse();
			self.rot *= -1;
			return self;
		}

		/**
		 * @brief Local To Parent
		 * @param self To Trans
		 * @param parentRef Parent Frame Reference Trans
		 * @return Transformed Translation
		 */
		[[nodiscard]] constexpr friend Transform operator|(Transform self, const Transform& parentRef){
			return self |= parentRef;
		}

		[[nodiscard]] constexpr friend Geom::Vec2& operator|=(Geom::Vec2& vec, const Transform& transform){
			return vec.rotate(transform.rot).add(transform.vec);
		}

		[[nodiscard]] constexpr friend Geom::Vec2 operator|(Geom::Vec2 vec, const Transform& transform){
			return vec |= transform;
		}
	};
}
