module ;

export module Event;

import <functional>;
import <typeindex>;
import <unordered_map>;
import Concepts;

export namespace Event {
	struct EventType {};

	struct Draw_After                   final : EventType {};
	struct Draw_Post                    final : EventType {};
	struct Draw_Prepare                 final : EventType {};
	struct AssertLoad_Prepare           final : EventType {};
	struct AssertLoad_Completed         final : EventType {};
	struct ApplicationMainLoop_Pre      final : EventType {};
	struct ApplicationMainLoop_After    final : EventType {};

	class EventManager {
	protected:
		std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> events;

	public:
		template <typename T>
			requires Concepts::Derived<T, EventType>
		void fire(const T& event) {
			if (const auto itr = events.find(std::type_index(typeid(T))); itr != events.end()) {
				for (const auto& listener : itr->second) {
					listener(&event);
				}
			}
		}

		template <typename T, typename Func>
			requires Concepts::Derived<T, EventType> && Concepts::Invokable<Func, void(const T&)>
		void on(Func&& func){
			const auto eventType = std::type_index(typeid(T));
			events[eventType].emplace_back([fun = std::forward<Func>(func)](const void* event) {
				fun(*static_cast<const T*>(event));
			});
		}
	};


	template <Concepts::Enum T, T maxsize>
	class EventManager_Quick {
	protected:
		std::array<std::vector<std::function<void()>>, static_cast<size_t>(maxsize)> events;

	public:
		void fire(const T event) {
			for (const auto tasks = events.at(static_cast<size_t>(event)); const auto& listener : tasks) {
				listener(&event);
			}
		}

		template <Concepts::Invokable<void()> Func>
		void on(const T index, Func&& func){
			events[static_cast<size_t>(index)].emplace_back(func);
		}
	};

	//TODO Move these to other places
	inline EventManager generalCheckEvents{};
	inline EventManager generalUpdateEvents{};
}

