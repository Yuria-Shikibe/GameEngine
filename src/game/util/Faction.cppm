module;

export module Game.Faction;

import <string>;
import <unordered_map>;
import <unordered_set>;

import Graphic.Color;
import GL.Texture.TextureRegionRect;

export namespace Game {
	class Faction;

	using FactionID = unsigned int;

	std::unordered_map<FactionID, Faction> globalFactions{};

	class Faction {
		FactionID id{0};
		std::string_view name{"undefind"};

		Graphic::Color color{};
		GL::TextureRegionRect* factionIcon{nullptr};

	public:
		[[nodiscard]] Faction() = default;

		[[nodiscard]] Faction(const FactionID id, const std::string_view name, const Graphic::Color& color)
			: id(id),
			name(name),
			color(color) {
		}

		[[nodiscard]] explicit Faction(const FactionID id)
			: id(id) {
		}

		[[nodiscard]] FactionID getID() const {
			return id;
		}
	};

	class FactionData {
		Faction* faction{nullptr};

		bool activated{false};

		std::unordered_set<FactionID> ally{};
		std::unordered_set<FactionID> hostile{};
		std::unordered_set<FactionID> neutral{};

	public:
		[[nodiscard]] bool isAllyTo(const FactionID id) const {
			return ally.contains(id);
		}

		[[nodiscard]] bool isHostileTo(const FactionID id) const {
			return hostile.contains(id);
		}

		[[nodiscard]] bool isNeutralTo(const FactionID id) const {
			return neutral.contains(id);
		}

		[[nodiscard]] bool isAllyTo(const Faction* t) const {
			return ally.contains(t->getID());
		}

		[[nodiscard]] bool isHostileTo(const Faction* t) const {
			return hostile.contains(t->getID());
		}

		[[nodiscard]] bool isNeutralTo(const Faction* t) const {
			return neutral.contains(t->getID());
		}

		//...properties...
	};
}
