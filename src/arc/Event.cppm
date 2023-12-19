module ;

export module Event;

import <functional>;
import <typeindex>;
import <unordered_map>;
import <set>;
import <array>;
import RuntimeException;
import Concepts;

export namespace Event {
	struct EventType {};

	template<Concepts::Derived<EventType> T>
	constexpr std::type_index indexOf() {
		return std::type_index(typeid(T));
	}

	class EventManager {
		std::set<std::type_index> registered{};
		std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> events;

	public:
		template <Concepts::Derived<EventType> T>
			requires std::is_final_v<T>
		void fire(const T& event) const {
#ifdef DEBUG_LOCAL
			if(!registered.contains(indexOf<T>()))throw ext::RuntimeException{"Unexpected Event Type!"};
#endif

			if (const auto itr = events.find(indexOf<T>()); itr != events.end()) {
				for (const auto& listener : itr->second) {
					listener(&event);
				}
			}
		}

		template <typename T, typename Func>
			requires Concepts::Derived<T, EventType> && Concepts::Invokable<Func, void(const T&)> && std::is_final_v<T>
		void on(Func&& func){
#ifdef DEBUG_LOCAL
			if(!registered.contains(std::type_index(typeid(T))))throw ext::RuntimeException{"Unexpected Event Type!"};
#endif

			const auto eventType = std::type_index(typeid(T));
			events[eventType].emplace_back([fun = std::forward<Func>(func)](const void* event) {
				fun(*static_cast<const T*>(event));
			});
		}

		template <Concepts::Derived<EventType> ...T>
		void registerType() {
			(registered.insert(std::type_index(typeid(T))), ...);
		}

		[[nodiscard]] explicit EventManager(std::set<std::type_index>&& registered)
			: registered(std::move(registered)) {
		}

		[[nodiscard]] explicit EventManager(const std::initializer_list<std::type_index> registeredList)
			: registered(registeredList) {
		}

		[[nodiscard]] EventManager() = default;
	};

	enum class CycleSignalState {
		begin, end,
		/** \brief Internal Index Count Usage*/
		maxCount
	};

	/**
	 * \brief THE VALUE OF THE ENUM MEMBERS MUST BE CONTINUOUS
	 * \tparam T Used Enum Type
	 * \tparam maxsize How Many Items This Enum Contains
	 */
	template <Concepts::Enum T, T maxsize>
	class SignalManager {
		std::array<std::vector<std::function<void()>>, static_cast<size_t>(maxsize)> events{};

	public:
		void fire(const T event) {
			for (const auto tasks = events.at(static_cast<size_t>(event)); const auto& listener : tasks) {
				listener();
			}
		}

		template <Concepts::Invokable<void()> Func>
		void on(const T index, Func&& func){
			events[static_cast<size_t>(index)].emplace_back(func);
		}
	};

	//TODO Move these to other places
	struct Draw_After   final : EventType {};
	struct Draw_Post    final : EventType {};
	struct Draw_Prepare final : EventType {};
	inline EventManager generalUpdateEvents{
		indexOf<Draw_After>(),
		indexOf<Draw_Post>(),
		indexOf<Draw_Prepare>()
	};

	using CycleSignalManager = Event::SignalManager<Event::CycleSignalState, Event::CycleSignalState::maxCount>;
}

