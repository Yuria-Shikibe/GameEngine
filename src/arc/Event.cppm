export module Event;

import std;
import ext.RuntimeException;
import ext.Concepts;

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
		std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> events{};

	public:
		template <Concepts::Derived<EventType> T>
			requires std::is_final_v<T>
		void fire(const T& event) const {
#ifdef _DEBUG
			checkRegister<T>();
#endif
			if (const auto itr = events.find(indexOf<T>()); itr != events.end()) {
				for (const auto& listener : itr->second) {
					listener(&event);
				}
			}
		}

		template <Concepts::Derived<EventType> T>
			requires std::is_final_v<T>
		void on(Concepts::Invokable<void(const T&)> auto&& func){
#ifdef _DEBUG
			checkRegister<T>();
#endif
			events[indexOf<T>()].emplace_back([fun = std::forward<decltype(func)>(func)](const void* event) {
				fun(*static_cast<const T*>(event));
			});
		}

		template <Concepts::Derived<EventType> ...T>
		void registerType() {
#ifdef _DEBUG
			(registered.insert(indexOf<T>()), ...);
#endif
		}

		[[nodiscard]] explicit EventManager(std::set<std::type_index>&& registered)
#ifdef _DEBUG
			: registered(std::move(registered))
#endif
		{}

		[[nodiscard]] EventManager(const std::initializer_list<std::type_index> registeredList)
#ifdef _DEBUG
			: registered(registeredList)
#endif
		{}

		[[nodiscard]] EventManager() = default;
	protected:
		template <Concepts::Derived<EventType> T>
		void checkRegister() const {
#ifdef _DEBUG
			if(!registered.empty() && !registered.contains(indexOf<T>()))throw ext::RuntimeException{"Unexpected Event Type!"};
#endif
		}

	};

	/**
	 * @brief THE VALUE OF THE ENUM MEMBERS MUST BE CONTINUOUS
	 * @tparam T Used Enum Type
	 * @tparam maxsize How Many Items This Enum Contains
	 */
	template <Concepts::Enum T, std::underlying_type_t<T> maxsize>
	class SignalManager {
		std::array<std::vector<std::function<void()>>, maxsize> events{};

	public:
		using SignalType = std::underlying_type_t<T>;
		template <T index>
		static constexpr bool isValid = requires{
			requires static_cast<SignalType>(index) < maxsize;
			requires static_cast<SignalType>(index) >= 0;
		};

		void fire(const T signal) {
			for (const auto& listener : events[static_cast<SignalType>(signal)]) {
				listener();
			}
		}

		void on(const T signal, Concepts::Invokable<void(const T&)> auto&& func){
			events[static_cast<SignalType>(signal)].emplace_back(std::forward<decltype(func)>(func));
		}

		template <T signal>
			requires isValid<signal>
		void on(Concepts::Invokable<void(const T&)> auto&& func){
			events[static_cast<SignalType>(signal)].emplace_back(std::forward<decltype(func)>(func));
		}

		template <T signal>
			requires isValid<signal>
		void fire(){
			for (const auto& listener : events[static_cast<SignalType>(signal)]) {
				listener();
			}
		}
	};


	enum class CycleSignalState {
		begin, end,
	};

	using CycleSignalManager = Event::SignalManager<Event::CycleSignalState, 2>;

	using enum CycleSignalState;
}

