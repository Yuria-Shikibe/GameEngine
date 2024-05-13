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

		constexpr Transform& operator|=(const Transform parentRef){
			vec.rotate(parentRef.rot).add(parentRef.vec);
			rot += parentRef.rot;

			return *this;
		}

		constexpr Transform& operator+=(const Transform other){
			vec += other.vec;
			rot += other.rot;

			return *this;
		}

		constexpr Transform& operator-=(const Transform other){
			vec -= other.vec;
			rot -= other.rot;

			return *this;
		}

		constexpr Transform& operator*=(const float scl){
			vec *= scl;
			rot *= scl;

			return *this;
		}

		[[nodiscard]] constexpr friend Transform operator*(Transform self, const float scl){
			return self *= scl;
		}

		[[nodiscard]] constexpr friend Transform operator+(Transform self, const Transform other){
			return self += other;
		}

		[[nodiscard]] constexpr friend Transform operator-(Transform self, const Transform other){
			return self -= other;
		}

		/**
		 * @brief Local To Parent
		 * @param self To Trans
		 * @param parentRef Parent Frame Reference Trans
		 * @return Transformed Translation
		 */
		[[nodiscard]] constexpr friend Transform operator|(Transform self, const Transform parentRef){
			return self |= parentRef;
		}
	};
}
