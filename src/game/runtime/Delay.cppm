module;

export module Game.Delay;

import ext.Concepts;
import Math.Timed;
import std;
import Core.Unit;

export namespace Game {
	using TickRatio = std::ratio<1, 60>;

	enum struct Priority{
		unignorable,
		important,
		normal,
		minor,

		last
	};

	struct DelayAction {
		Math::Timed progress{};

		unsigned int repeatCount{};
		unsigned int repeatedCount{};

		std::function<void()> action{nullptr};

		[[nodiscard]] DelayAction() = default;


		[[nodiscard]] DelayAction(const Core::Tick tick, Concepts::Invokable<void()> auto&& func)
			: progress(0, tick.count()), action{func} {
		}

		[[nodiscard]] DelayAction(const Core::Tick tick, std::function<void()>&& func)
			: progress(0, tick.count()), action{std::move(func)} {
		}

		[[nodiscard]] DelayAction(const Core::Tick tick, const unsigned repeat, Concepts::Invokable<void()> auto&& func)
			: progress(0, tick.count()), repeatCount{repeat}, action{func} {
		}

		[[nodiscard]] DelayAction(const Core::Tick tick, const unsigned repeat, std::function<void()>&& func)
			: progress(0, tick.count()), repeatCount{repeat}, action{std::move(func)} {
		}


		constexpr bool hasRepeat() const noexcept{
			return repeatCount > 0;
		}

		bool update(const Core::Tick deltaTick) {
			progress.update(deltaTick);

			if(progress) {
				this->operator()();
				if(repeatCount - repeatedCount > 0){
					repeatedCount++;
					progress.time = 0;
					return false;
				}else{
					return true;
				}
			}

			return false;
		}

		void operator()() const{
			action();
		}
	};

	class DelayActionManager {
		Priority lowestPriority = Priority::last;

		using PriorityIndex = std::underlying_type_t<Priority>;

		struct ActionGroup{
			std::vector<DelayAction> actives{};
			std::vector<DelayAction> toAdd{};
			std::mutex mtx{};
		};

		std::array<ActionGroup, static_cast<std::size_t>(Priority::last) + 1> taskGroup{};

		auto& getGroupAt(const Priority priority){
			return taskGroup[static_cast<PriorityIndex>(priority)];
		}

		auto& getGroupAt(const Priority priority) const{
			return taskGroup[static_cast<PriorityIndex>(priority)];
		}

	public:
		[[nodiscard]] DelayActionManager() {
			for (auto & [actives, toAdd, _] : taskGroup){
				actives.reserve(500);
				toAdd.reserve(500);
			}
		}

		[[nodiscard]] constexpr Priority getPriority() const noexcept{ return lowestPriority; }

		constexpr void setPriority(const Priority lowestPriority) noexcept{ this->lowestPriority = lowestPriority; }

		template <Concepts::Invokable<void()> Func>
		void launch(const Priority priority, Core::Tick tick, unsigned count, Func&& action) {
			if(priority > lowestPriority)return;

			auto& group = getGroupAt(priority);
			std::lock_guard lockGuard{group.mtx};
			group.toAdd.emplace_back(tick, count, action);
		}

		void clear() {
			for (auto & [actives, toAdd, _] : taskGroup){
				actives.clear();
				toAdd.clear();
			}
		}

		void update(Core::Tick deltaTick) {
			for(auto& [actives, toAdd, mtx] : std::ranges::subrange{
				    taskGroup.begin(), taskGroup.begin() +
				    	(static_cast<PriorityIndex>(lowestPriority) + 1)
			    }){
				std::lock_guard lockGuard{mtx};

				actives.reserve(actives.size() + toAdd.size());
				std::ranges::move(std::move(toAdd), std::back_inserter(actives));
				toAdd.clear();

				std::erase_if(actives, [deltaTick](DelayAction& action){
					return action.update(deltaTick);
				});
			}
		}
	};
}
