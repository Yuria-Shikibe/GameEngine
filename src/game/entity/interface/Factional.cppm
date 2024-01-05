//
// Created by Matrix on 2023/12/31.
//

export module Game.Entity.Factional;

import Game.Entity;

export import Game.Faction;
export import Game.Factions;

export namespace Game {
	class Factional{
	protected:
		Faction* faction = &Factions::ownerless;

	public:
		virtual ~Factional() = default;

		[[nodiscard]] virtual Faction* getFaction() const {
			return faction;
		}

		virtual void setFaction(Faction* const faction) {
			this->faction = faction;
		}
	};
}
