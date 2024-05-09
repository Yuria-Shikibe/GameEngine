export module Game.Entity.Factional;

import Game.Entity;

export import Game.Faction;
export import Game.Factions;

export namespace Game {
	class Factional{
	protected:
		Faction* faction = &Factions::ownerless;

	public:
		[[nodiscard]] Factional() = default;

		virtual ~Factional() = default;

		[[nodiscard]] virtual Faction* getFaction() const noexcept{
			return faction;
		}

		virtual void setFaction(Faction* const faction) noexcept{
			if(faction){
				this->faction = faction;
			}else{
				this->faction = &Factions::ownerless;
			}
		}
	};
}
