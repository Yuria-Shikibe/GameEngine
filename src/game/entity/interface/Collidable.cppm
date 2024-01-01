module;

export module Game.Entity.Collidable;

export namespace Game {
	class Collidable {
	public:
		virtual ~Collidable() = default;

		virtual bool ignoreCollision() {
			return false;
		}

		//...
	};
}
