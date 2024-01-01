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
#ifdef _DEBUG
		std::set<std::type_index> registered{};
#endif
		std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> events;

	public:
		template <Concepts::Derived<EventType> T>
			requires std::is_final_v<T>
		void fire(const T& event) const {
#ifdef _DEBUG
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
#ifdef _DEBUG
			if(!registered.contains(std::type_index(typeid(T))))throw ext::RuntimeException{"Unexpected Event Type!"};
#endif
			const auto eventType = std::type_index(typeid(T));
			events[eventType].emplace_back([fun = std::forward<Func>(func)](const void* event) {
				fun(*static_cast<const T*>(event));
			});
		}

		template <Concepts::Derived<EventType> ...T>
		void registerType() {
#ifdef _DEBUG
			(registered.insert(std::type_index(typeid(T))), ...);
#endif
		}

		[[nodiscard]] explicit EventManager(std::set<std::type_index>&& registered)
#ifdef _DEBUG
			: registered(std::move(registered))
#endif
		{}

		[[nodiscard]] explicit EventManager(const std::initializer_list<std::type_index> registeredList)
#ifdef _DEBUG
			: registered(registeredList)
#endif
		{}

		[[nodiscard]] EventManager() = default;
	};

	/**
	 * \brief THE VALUE OF THE ENUM MEMBERS MUST BE CONTINUOUS
	 * \tparam T Used Enum Type
	 * \tparam maxsize How Many Items This Enum Contains
	 */
	template <Concepts::Enum T, size_t maxsize>
	class SignalManager {
		std::array<std::vector<std::function<void()>>, maxsize> events{};

	public:
		void fire(const T event) {
			for (const auto tasks = events.at(static_cast<size_t>(event)); const auto& listener : tasks) {
				listener();
			}
		}

		template <Concepts::Invokable<void()> Func>
		void on(const T index, Func&& func){
			events[static_cast<size_t>(index)].emplace_back(std::forward<Func>(func));
		}
	};


	enum class CycleSignalState {
		begin, end,
	};

	using CycleSignalManager = Event::SignalManager<Event::CycleSignalState, 2>;

	using enum CycleSignalState;
}

