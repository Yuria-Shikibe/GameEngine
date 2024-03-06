module;

export module Game.Entity.PhysicsAttribute;

export namespace Game {
	struct PhysicsAttribute_Rigid{
		float inertialMass = 1000;
		float rotationalInertiaScale = 1 / 12.0f;
		/** [0, 1]*/
		float frictionCoefficient = 0.25f;
		float restitution = 0.0f;
		/** Used For Force Correction*/
		float collideForceScale = 1.0f;
	};
}
