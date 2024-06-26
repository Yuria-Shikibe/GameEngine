export module ext.Event;

import std;
import ext.RuntimeException;
import ext.Concepts;

export namespace ext {
	struct EventType {};

	template<Concepts::Derived<EventType> T>
	constexpr std::type_index typeIndexOf() {
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
			if (const auto itr = events.find(typeIndexOf<T>()); itr != events.end()) {
				for (const auto& listener : itr->second) {
					listener(&event);
				}
			}
		}

		template <Concepts::Derived<EventType> T, typename ...Args>
			requires std::is_final_v<T>
		void emplace_fire(Args&& ...args) const {
			this->fire<T>(T{std::forward<Args>(args) ...});
		}

		template <Concepts::Derived<EventType> T, std::invocable<const T&> Func>
			requires std::is_final_v<T>
		void on(Func&& func){
#ifdef _DEBUG
			checkRegister<T>();
#endif
			events[typeIndexOf<T>()].emplace_back([fun = std::forward<decltype(func)>(func)](const void* event) {
				fun(*static_cast<const T*>(event));
			});
		}

		template <Concepts::Derived<EventType> ...T>
		void registerType() {
#ifdef _DEBUG
			(registered.insert(typeIndexOf<T>()), ...);
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
			if(!registered.empty() && !registered.contains(typeIndexOf<T>()))throw ext::RuntimeException{"Unexpected Event Type!"};
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

		[[nodiscard]] static constexpr SignalType max() noexcept{
			return maxsize;
		}

		void fire(const T signal) {
			for (const auto& listener : events[std::to_underlying(signal)]) {
				listener();
			}
		}

		template <std::invocable Func>
		void on(const T signal, Func&& func){
			events.at(std::to_underlying(signal)).push_back(std::function<void()>{std::forward<Func>(func)});
		}

		template <T signal, std::invocable Func>
			requires isValid<signal>
		void on(Func&& func){
			events.at(std::to_underlying(signal)).push_back(std::function<void()>{std::forward<Func>(func)});
		}

		template <T signal>
			requires isValid<signal>
		void fire(){
			for (const auto& listener : events[std::to_underlying(signal)]) {
				listener();
			}
		}
	};


	enum class CycleSignalState {
		begin, end,
	};

	using CycleSignalManager = ext::SignalManager<ext::CycleSignalState, 2>;

	using enum CycleSignalState;
}

