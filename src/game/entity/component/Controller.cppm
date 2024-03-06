module;

export module Game.Entity.Controller;

import Game.Logic.Strategy;
import Game.Logic.Objective;
import Game.Logic.ReflectSensor;

import <functional>;
import <memory>;
import <vector>;
import <queue>;

import Geom.Vector2D;

export namespace Game {
	using HittableTarget = std::weak_ptr<RealityEntity>;
	using PosedTarget = Geom::Vec2;

	struct MoveCommand {
		/**
		 * \brief Relative Ctrl Position Source
		 */
		PosedTarget motionRelativeSrc{};
		/**
		 * \brief Displacement To Source
		 */
		PosedTarget motionRelativeDst{};
		/**
		 * \brief Axis of the Source, should has the same angle with its face direction
		 * TODO uses a single float instead
		 * TODO should this in world frame reference?
		 */
		PosedTarget motionRelativeDir{};

		PosedTarget destination{};

		bool enableCommand{false};

		//Should This Be Queue? or uses this index mode
		int currentRouteIndex{0};
		std::vector<PosedTarget> route{};

		Geom::Vec2 currentPosition{};

		/** \brief What velocity should this entity hold. */
		Geom::Vec2 expectedVelocity{};

		Geom::Vec2 expectedAcceleration{};

		/** \brief What angle should this entity at. */
		float expectedFaceAngle{};

		/** \brief In what angle should this entity move. */
		[[nodiscard]] float expectedMoveAngle() const {
			return expectedVelocity.angle();
		}

		void updateCurrent(const Geom::Vec2 data) {
			currentPosition = data;
		}

		void activate() {
			enableCommand = true;
		}

		[[nodiscard]] bool activated() const {
			return enableCommand;
		}
	};

	/**
	 * \brief Should be thread independent and parallarable.
	 */
	class Controller {
	public:
		virtual ~Controller() = default;

		std::vector<HittableTarget> targets{};

		MoveCommand moveCommand{};

		bool selected{false};

	protected:
		/** \brief Command modify it's strategy. This is only a const reference*/
		std::unique_ptr<Strategy> strategy{nullptr};
		/** \brief Command shouldn't modify it's commanded entity directly. */
		const RealityEntity* owner{nullptr};

	public:
		/**
		 * \brief
		 * What should this entity to do.
		 *		such as mining, attacking, defensing...
		 * It should be noticed that the most important objective should at the back of the vector
		 */
		std::vector<std::unique_ptr<Objective>> objectives{};

		/**
		 * \brief TODO should this sustain in here?
		 */
		ReflectSensor* sensor{nullptr};

		[[nodiscard]] explicit Controller(const RealityEntity* const owner)
			: owner(owner) {
		}

		virtual bool isValidTo(std::shared_ptr<RealityEntity> entity) {
			return true;
		}

		virtual bool hasValidTarget() {
			return std::ranges::any_of(std::as_const(targets), [this](const decltype(targets)::value_type& target) {
				return !target.expired() && isValidTo(target.lock());
			});
		}

		virtual void changeStrategy(Strategy* strategy) {
			this->strategy = strategy->copy(owner);

			strategy->optimizeObjectives(objectives);
			strategy->optimizeTarget(targets);
		}

		virtual void findTarget() {

		}

		//TODO Should this be moved to Controller?
		virtual void updateTarget() {
			findTarget();

			strategy->optimizeTarget(targets);
		}

		virtual void update(/*const float delta*/);

		virtual void postObjective(std::unique_ptr<Objective>&& objective) {
			strategy->postObjective(objectives, std::forward<std::unique_ptr<Objective>>(objective));
		}
	};
}
