//
// Created by Matrix on 2024/3/15.
//

export module Game.Content.Drawer.DrawParam;

export import Geom.Transform;

export import Game.Entity.BaseEntity;

export namespace Game::Drawer{
	struct CompPos : Geom::Transform{
		float zOffset{};

		constexpr CompPos() = default;

		explicit CompPos(const Geom::Transform trans, const float zOffset)
			: Geom::Transform{trans}, zOffset(zOffset){}

		constexpr CompPos(const Geom::Vec2 pos, const float rot, const float zOffset)
			: Geom::Transform(pos, rot),
			  zOffset(zOffset){}

		constexpr CompPos& operator|=(const CompPos parentRef){
			Geom::Transform::operator|=(static_cast<Geom::Transform>(parentRef)); // NOLINT(*-slicing)
			zOffset += parentRef.zOffset;

			return *this;
		}

		/**
		 * @brief Local To Parent
		 * @param self To Trans
		 * @param parentRef Parent Frame Reference Trans
		 * @return Transformed Translation
		 */
		[[nodiscard]] constexpr friend CompPos operator|(CompPos self, const CompPos parentRef){
			return self |= parentRef;
		}

		constexpr CompPos& operator+=(const CompPos parentRef){
			Geom::Transform::operator+=(static_cast<Geom::Transform>(parentRef)); // NOLINT(*-slicing)
			zOffset += parentRef.zOffset;

			return *this;
		}

		[[nodiscard]] constexpr CompPos operator*(const float scl) const{
			CompPos state{*this};
			state.Geom::Transform::operator*=(scl);
			state.zOffset *= scl;

			return state;
		}
	};

	[[nodiscard]] CompPos getCompPos(const BaseEntity* entity){
		return CompPos{entity->trans, entity->zLayer};
	}

	struct DrawProgress{
		float lifetimeProgress{};
		float reloadProgress{};

		//TODO Impl In Future
		float chargeProgress{};
		float thursterProgress{};
	};

	struct DrawParam {
		/**
		 * @brief
		 * Parent Position
		 * Parent Rotation
		 * Parent Z
		 */
		CompPos trans{};
		DrawProgress progress{};
	};


}
