module;

export module Game.Entity;

import Game.Entity.RemoveCallalble;
import <atomic>;
import <memory>;
import <string_view>;
import <vector>;
import <limits>;
import RuntimeException;
import Geom.Shape.Rect_Orthogonal;
import Event;


export namespace Game {
	using IDType = unsigned int;
	using SerializationIDType = unsigned int;
	using SignatureType = size_t;

	class Entity;

	class RemoveCallalble {
	public:
		virtual ~RemoveCallalble() = default;

		virtual void postRemovePrimitive(Game::Entity* entity) = 0;

		virtual void postAddPrimitive(Game::Entity* entity) = 0;
	};

	class Entity : public std::enable_shared_from_this<Entity> {
	public:
		virtual ~Entity() = default;

		static constexpr SerializationIDType SerializationDisabled = std::numeric_limits<SerializationIDType>::max();

	protected:
		SerializationIDType serializationID{SerializationDisabled};
		IDType id{0};
		SignatureType signature{0};
		std::atomic_bool activated{false};
		bool sleeping{false};

		std::vector<RemoveCallalble*> sharedGroup{};

		Event::EventManager listener{};

	public:
		[[nodiscard]] bool serializable() const {
			return serializationID != SerializationDisabled;
		}

		void setSerializationID(const SerializationIDType serializationID = SerializationDisabled) {
			this->serializationID = serializationID;
		}

		[[nodiscard]] Event::EventManager& getListener() {
			return listener;
		}

		//TODO io support

		virtual void update(float deltaTick/*should Delta applied here or globally?*/) = 0;

		[[nodiscard]] virtual bool deletable() const{
			return !activated;
		}

		virtual void informAllReferencer() {
			//Sustain this currently
		}

		virtual std::string_view getClassname() {
			return typeid(*this).name();
		}

		virtual void activate() {
			activated = true;
		}

		[[nodiscard]] virtual bool valid() const{
			return activated;
		}

		virtual void deactivate() {
			if(!activated)return;
			activated = false;
			for(const auto& group : sharedGroup) {
				group->postRemovePrimitive(this);
			}
			sharedGroup.clear();
		}

		virtual void registerGroup(RemoveCallalble* callalble) {
			sharedGroup.push_back(callalble);
		}

		[[nodiscard]] virtual bool isSleeping() {
			return sleeping;
		}

		[[nodiscard]] virtual IDType getID() const {
			return id;
		}

		virtual void setID(const IDType id) {
			if(activated)throw ext::RuntimeException{"Setting An Activated Entity's ID is banned!"};

			this->id = id;
		}

		/**
		 * \brief Nullptr condition should be considered!
		 */
		[[nodiscard]] virtual std::shared_ptr<Entity> obtainSharedSelf() {
			if(activated) {
				return shared_from_this();
			}
			return weak_from_this().lock();
		}

		/**
		 * \brief Nullptr condition should be considered!
		 */
		[[nodiscard]] virtual std::shared_ptr<const Entity> obtainSharedSelf() const {\
			if(activated) {
				return shared_from_this();
			}
			return weak_from_this().lock();
		}
	};
}

export {
	template<>
	struct std::hash<Game::Entity>{
		size_t operator()(const Game::Entity& entity) const noexcept {
			return entity.getID();
		}
	};

	template<Concepts::Derived<Game::Entity> K, typename V>
	struct std::hash<std::pair<K*, V>>{
		size_t operator()(const std::pair<K*, V>& entity) const noexcept {
			return entity.first->getID();
		}
	};
}
