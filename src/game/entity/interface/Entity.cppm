module;

export module Game.Entity;

import <limits>;
import RuntimeException;
import Geom.Shape.Rect_Orthogonal;
import Event;

export namespace Game {
	using IDType = unsigned int;
	using SerializationIDType = unsigned int;
	using SignatureType = size_t;

	class Entity {
	public:
		virtual ~Entity() = default;

		static constexpr SerializationIDType SerializationDisabled = std::numeric_limits<SerializationIDType>::max();

	protected:
		SerializationIDType serializationID{SerializationDisabled};
		IDType id{0};
		SignatureType signature{0};
		bool activated{false};
		bool sleeping{false};

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

		virtual bool deletable(){
			return !activated;
		}

		virtual void informAllReferencer() {
			//Sustain this currently
		}

		virtual void activate() = 0;

		virtual void deactivate() = 0;

		virtual bool isSleeping() {
			return sleeping;
		}

		[[nodiscard]] virtual IDType getID() const {
			return id;
		}

		virtual void setID(const IDType id) {
			if(activated)throw ext::RuntimeException{"Setting An Activated Entity's ID is banned!"};

			this->id = id;
		}
	};
}
