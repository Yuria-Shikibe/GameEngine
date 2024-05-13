module;

export module Game.Entity.Controller;

import Game.Logic.Strategy;
import Game.Logic.Objective;
import Game.Logic.ReflectSensor;

import std;
import Math;

import Geom.Transform;

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

		bool enableMoveCommand{false};
		bool enableRotateCommand{false};

		bool translatory{false};

		bool overrideControl{false};

		float positionTolerance{32.0f};

		//Should This Be Queue? or uses this index mode
		int currentRouteIndex{0};
		std::vector<PosedTarget> route{};

		Geom::Transform curTrans{};

		/** \brief What velocity should this entity hold. */
		Geom::Vec2 expectedVelocity{};

		Geom::Vec2 expectedAcceleration{};

		/** \brief What angle should this entity at. */
		float expectedFaceAngle{};

		/** \brief In what angle should this entity move. */
		[[nodiscard]] float expectedMoveAngle() const {
			return expectedVelocity.angle();
		}

		[[nodiscard]] Geom::Vec2 lastDest() const{
			const auto currentRouteIndex = this->currentRouteIndex - 1;
			if(currentRouteIndex < 0){
				return destination;
			}
			if(currentRouteIndex >= route.size()){
				return destination;
			}

			return route.at(currentRouteIndex);
		}

		[[nodiscard]] Geom::Vec2 nextDest() const{
			if(currentRouteIndex < 0){
				return destination;
			}
			if(currentRouteIndex >= route.size()){
				return destination;
			}

			return route.at(currentRouteIndex);
		}

		[[nodiscard]] bool hasNextDest() const{
			return currentRouteIndex >= 0 && currentRouteIndex < route.size();
		}

		void setRouteAssigningEnd(){
			currentRouteIndex = 0;
		}

		void setRouteAssigningBegin(){
			deactivateMove();
			currentRouteIndex = -1;
		}

		[[nodiscard]] bool isAssigningRoute() const{
			return currentRouteIndex < 0;
		}

		[[nodiscard]] bool hasRoute() const{
			return isAssigningRoute() || !route.empty();
		}

		[[nodiscard]] bool requiresMovement(const Geom::Vec2 next) const{
			return (curTrans.vec - next).length2() > positionTolerance * positionTolerance;
		}

		void updateCurrent(const Geom::Transform current) {
			curTrans = current;

			if(translatory){

			}else if(moveActivated()){
				expectedFaceAngle = expectedMoveAngle();
			}

			if(overrideControl)return;

			const auto dest = nextDest();

			expectedVelocity.set(dest - curTrans.vec).setLength(30);



			if(Math::Angle::angleDst(curTrans.rot, expectedFaceAngle) > 0.005f){
				activateRotate();
			}else{
				completeRotate();
			}

			if(moveActivated()){
				if(!requiresMovement(dest)){
					currentRouteIndex ++;
					if(!hasNextDest()){
						completeMove();
					}
				}
			}
		}

		void activateRotate() {
			enableRotateCommand = true;
		}

		void deactivateRotate() {
			enableRotateCommand = false;
		}

		void activateMove() {
			enableMoveCommand = true;
		}

		void deactivateMove() {
			enableMoveCommand = false;
		}

		void completeMove(){
			destination = curTrans.vec + expectedVelocity * 2;
			clearRoute();
			deactivateMove();
		}

		void completeRotate(){
			expectedFaceAngle = curTrans.rot;
			deactivateRotate();
		}

		void clearRoute(){
			route.clear();
			currentRouteIndex = 0;
		}

		void assignTarget(const Geom::Vec2 dest){
			destination = dest;
			activateMove();
		}

		[[nodiscard]] bool moveActivated() const {
			return enableMoveCommand;
		}

		[[nodiscard]] bool rotateActivated() const {
			return enableRotateCommand;
		}

		[[nodiscard]] bool shouldDrawUI() const{
			return enableMoveCommand || enableRotateCommand || isAssigningRoute();
		}
	};

	/**
	 * \brief Should be thread independent and parallarable.
	 */
	class Controller {
	public:
		virtual ~Controller() = default;

		std::vector<HittableTarget> targets{};

		std::vector<Geom::Vec2> turretTargets{};

		MoveCommand moveCommand{};

		bool selected{false};

	protected:
		/** \brief Command modify it's strategy. This is only a const reference*/
		std::unique_ptr<Strategy> strategy{nullptr};
		/** \brief Command shouldn't modify it's commanded entity directly. */
		RealityEntity* owner{nullptr};

		std::unique_ptr<Controller> fallbackController{nullptr};

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

		[[nodiscard]] explicit Controller(RealityEntity* const owner);

		virtual bool selectable() const {
			return true;
		}

		virtual bool isValidTo(const std::shared_ptr<RealityEntity>& entity) {
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

		virtual void assignTurretTarget(std::vector<Geom::Vec2>&& targets);

		virtual void findTarget() {

		}

		//TODO Should this be moved to Controller?
		virtual void updateTarget() {
			findTarget();

			if(strategy)strategy->optimizeTarget(targets);
		}

		virtual void update(/*const float delta*/);

		virtual void postObjective(std::unique_ptr<Objective>&& objective) {
			if(strategy)strategy->postObjective(objectives, std::forward<std::unique_ptr<Objective>>(objective));
		}
	};
}
