//
// Created by Matrix on 2024/3/21.
//

export module Geom.Transform;

export import Geom.Vector2D;

import Concepts;

export namespace Geom{
	struct Transform {
		Vec2 pos{};
		float rot{};

		constexpr Transform& operator|=(const Transform parentRef){
			pos.rotate(parentRef.rot).add(parentRef.pos);
			rot += parentRef.rot;

			return *this;
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

		constexpr Transform& operator+=(const Transform other){
			pos += other.pos;
			rot += other.rot;

			return *this;
		}

		constexpr Transform& operator*=(const float scl){
			pos *= scl;
			rot *= scl;

			return *this;
		}

		[[nodiscard]] constexpr Transform operator*(const float scl) const{
			Transform state{*this};
			state.pos *= scl;
			state.rot *= scl;

			return state;
		}
	};
}
