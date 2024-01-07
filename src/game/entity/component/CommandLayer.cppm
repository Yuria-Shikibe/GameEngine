module;

export module Game.Entity.CommandLayer;

import Game.Entity.RealityEntity;
import Game.Entity.PosedEntity;

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

	/**
	 * \brief Should be thread independent and parallarable.
	 */
	class CommandLayer {
	public:
		virtual ~CommandLayer() = default;

		std::vector<HittableTarget> targets{};

		PosedTarget motionRelative{};

		PosedTarget destination{};
		std::vector<Geom::Vec2> route{};

		Geom::Vec2 currentPosition{};

		/** \brief What velocity should this entity hold. */
		Geom::Vec2 expectedVelocity{};
		Geom::Vec2 expectedAcceleration{};

		/** \brief What angle should this entity in. */
		float expectedFaceAngle{};
		/** \brief What angle should this entity move, should be an output value of @link expectedVelocity @endlink. */
		float expectedMoveAngle{};

	protected:
		/** \brief Command modify it's strategy. This is only a const reference*/
		std::unique_ptr<Strategy> strategy{nullptr};
		/** \brief Command shouldn't modify it's commanded entity directly. */
		const RealityEntity* entity{nullptr};

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

		virtual bool hasValidTarget() {
			return std::ranges::any_of(std::as_const(targets), [](const decltype(targets)::value_type& target) {
				return !target.expired();
			});
		}

		virtual void changeStrategy(Strategy* strategy) {
			this->strategy = strategy->copy(entity);

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

		virtual void update(/*const float delta*/) {
			updateTarget();
		}

		virtual void postObjective(std::unique_ptr<Objective>&& objective) {
			strategy->postObjective(objectives, std::forward<std::unique_ptr<Objective>>(objective));
		}
	};
}
