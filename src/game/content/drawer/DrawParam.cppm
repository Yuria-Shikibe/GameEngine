//
// Created by Matrix on 2024/3/15.
//

export module Game.Content.Drawer.DrawParam;

export import Geom.Transform;

export import Game.Entity.BaseEntity;

export namespace Game::Drawer{
	struct PartTrans : Geom::Transform{
		float zOffset{};

		constexpr PartTrans() = default;

		explicit PartTrans(const Geom::Transform trans, const float zOffset)
			: Geom::Transform{trans}, zOffset(zOffset){}

		constexpr PartTrans(const Geom::Vec2 pos, const float rot, const float zOffset)
			: Geom::Transform(pos, rot),
			  zOffset(zOffset){}

		constexpr PartTrans& operator|=(const PartTrans parentRef){
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
		[[nodiscard]] constexpr friend PartTrans operator|(PartTrans self, const PartTrans parentRef){
			return self |= parentRef;
		}

		constexpr PartTrans& operator+=(const PartTrans parentRef){
			Geom::Transform::operator+=(static_cast<Geom::Transform>(parentRef)); // NOLINT(*-slicing)
			zOffset += parentRef.zOffset;

			return *this;
		}

		[[nodiscard]] constexpr PartTrans operator*(const float scl) const{
			PartTrans state{*this};
			state.Geom::Transform::operator*=(scl);
			state.zOffset *= scl;

			return state;
		}
	};

	[[nodiscard]] PartTrans getCompPos(const BaseEntity* entity){
		return PartTrans{entity->trans, entity->zLayer};
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
		PartTrans trans{};
		DrawProgress progress{};
	};


}
