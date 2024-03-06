module Game.Math.Physics.Collision;

import Game.Entity.RealityEntity;

void Math::Phy::calCollideTo(Game::RealityEntity* subject, const Game::RealityEntity* object, Geom::Vec2 intersection) {
	// Geom::Vec2 pointToCenter = subject->hitBox.originPoint;
	// pointToCenter -= intersection;
	//
	// Geom::Vec2 relativeVelo = object->velocity;
	// relativeVelo -= subject->velocity;
	// relativeVelo.add(
	// 	object->velocity.y * object->angularVelocity - subject->velocity.y * subject->angularVelocity,
	// 	subject->velocity.x * subject->angularVelocity - object->velocity.x * object->angularVelocity
	// );
	//
	// const float dot = relativeVelo.dot(pointToCenter);
	//
	// float j = -(1 + subject->collideForceScale) * dot /
	//           (1 / subject->inertialMass + 1 / object->inertialMass + dot / subject->inertialMass);
	//
	// subject->velocity.sub(pointToCenter, j / subject->inertialMass);
	// subject->angularVelocity -= j * dot / subject->inertialMass;
}
