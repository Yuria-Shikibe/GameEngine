module;

export module Game.Delay;

import ext.Container.ObjectPool;
import Concepts;

import std;

export namespace Game {
	using TickRatio = std::ratio<1, 60>;
	struct DelayAction {
		float tick{};
		std::function<void()> action{nullptr};

		[[nodiscard]] DelayAction() = default;

		[[nodiscard]] explicit DelayAction(const std::chrono::duration<float, TickRatio>& tick)
			: tick(tick.count()) {
		}

		bool update(const float deltaTick) {
			tick -= deltaTick;
			if(tick <= 0) {
				this->operator()();
				return true;
			}
			return false;
		}

		void operator()() {
			action();
			action = nullptr;
		}

		template <Concepts::Invokable<void()> Func>
		void set(const float tick, Func&& action) {
			this->tick = tick;
			this->action = std::forward<Func>(action);
		}

		template <typename T>
		[[nodiscard]] explicit DelayAction(const T& tick)
			: tick(std::chrono::duration_cast<std::chrono::duration<float, TickRatio>>(tick)) {
		}

		[[nodiscard]] DelayAction(const std::chrono::duration<float, TickRatio>& tick,
			std::function<void()>&& action)
			: tick(tick.count()),
			action(std::forward<decltype(action)>(action)) {
		}

		[[nodiscard]] DelayAction(const float tick,
			std::function<void()>&& action)
			: tick(tick),
			action(std::forward<decltype(action)>(action)) {
		}
	};

	class DelayActionManager {
		std::mutex lock{};
		ext::ObjectPool<DelayAction> actionPools{2000};

		std::vector<std::unique_ptr<DelayAction, decltype(actionPools)::Deleter>> delayedTasks{};
		decltype(delayedTasks) toAdd{};

	public:
		[[nodiscard]] DelayActionManager() {
			delayedTasks.reserve(500);
			toAdd.reserve(100);
		}

	private:
		template <Concepts::Invokable<void()> Func>
		void suspend(float tick, Func&& action) {
			DelayAction* ptr;

			{
				std::lock_guard lockGuard{lock};
				ptr = toAdd.emplace_back(actionPools.obtainUnique()).get();
			}

			ptr->set(tick, std::forward<Func>(action));
		}

		void clear() {
			delayedTasks.clear();
			toAdd.clear();
		}

		void update(float deltaTick) {
			delayedTasks.reserve(delayedTasks.size() + toAdd.size());

			std::ranges::move(std::move(toAdd), std::back_inserter(delayedTasks));
			toAdd.clear();

			size_t doneCount = 0;
			std::for_each(
				std::execution::par_unseq,
				delayedTasks.begin(), delayedTasks.end(),
				[deltaTick, &doneCount, this](decltype(delayedTasks)::value_type& t){
					if(t->update(deltaTick)) {
						t.reset(nullptr);
						doneCount++;
					}
			});

			if(delayedTasks.empty() || doneCount == 0)return;

			std::erase_if(delayedTasks,
			[this, &doneCount](const decltype(delayedTasks)::value_type& ptr) {
				if(doneCount == 0)return false;

				if(!static_cast<bool>(ptr)) {
					doneCount--;
					return true;
				}

				return false;
			});
		}
	};
}
