module;

export module Game.Logic.Strategy;

import <string>;
import <memory>;
import <functional>;
import <algorithm>;
import <vector>;
import <numbers>;

import Geom.Vector2D;
import Game.Logic.Objective;

export namespace Game {
	class RealityEntity;

	class Strategy {
	public:
		virtual ~Strategy() = default;

		std::string_view name{"Strategy Undefined"};
		const RealityEntity* self{nullptr};

		[[nodiscard]] std::string_view getName() const {
			return name;
		}

		void setName(const std::string_view name) {
			this->name = name;
		}

		[[nodiscard]] const RealityEntity* getSelf() const {
			return self;
		}

		void setSelf(const RealityEntity* const self) {
			this->self = self;
		}

		[[nodiscard]] virtual bool objectiveCmp(const Objective* l, const Objective* r) const = 0;

		[[nodiscard]] virtual bool targetCmp(const RealityEntity* l, const RealityEntity* r) const = 0;

		[[nodiscard]] virtual bool validObjective(const Objective* t) const {
			return true;
		}

		[[nodiscard]] virtual bool validTarget(const RealityEntity* t) const {
			return true;
		}

		virtual std::unique_ptr<Strategy> copy(const RealityEntity* t) const = 0;/*{
			auto ptr = std::make_unique<std::remove_reference_t<decltype(*this)>>(*this);
			ptr->setSelf(t);
			return ptr;
		}*/

		[[nodiscard]] virtual bool caresTargetPriority() const {
			return true;
		}

		virtual void postObjective(std::vector<std::unique_ptr<Objective>>& objectives, std::unique_ptr<Objective>&& objective) const {
			using ValueType = std::vector<std::unique_ptr<Objective>>::value_type;

			objectives.insert(
				std::ranges::lower_bound(
					std::as_const(objectives), objective, [this](const ValueType& l, const ValueType& r) {
						return objectiveCmp(l.get(), r.get());
					}),
				std::forward<std::unique_ptr<Objective>>(objective)
			);
		}

		virtual void optimizeTarget(std::vector<std::weak_ptr<RealityEntity>>& targets) const {
			using ItemType = std::vector<std::weak_ptr<RealityEntity>>::value_type;

			std::erase_if(targets, [this](const ItemType& target) {
				return target.expired() || !validTarget(target.lock().get());
			});

			if(caresTargetPriority()) {
				std::ranges::sort(targets, [this](const ItemType& l, const ItemType& r) {
					return targetCmp(l.lock().get(), r.lock().get());
				});
			}
		}

		virtual void optimizeObjectives(std::vector<std::unique_ptr<Objective>>& objectives) const {
			using ValueType = std::vector<std::unique_ptr<Objective>>::value_type;

			std::ranges::sort(objectives, [this](const ValueType& l, const ValueType& r) {
				return objectiveCmp(l.get(), r.get());
			});
		}
	};
}
