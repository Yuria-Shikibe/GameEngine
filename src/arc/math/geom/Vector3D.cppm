module ;

export module Geom.Vector3D;

export namespace Geom{
	class Vector3D
	{
	public:
		Vector3D(const float x, const float y, const float z)
				: x(x),
				  y(y),
				  z(z){
		}

		Vector3D(const float x, const float y) : Vector3D(x, y, 0){
			// std::size()
		}



		float x = 0, y = 0, z = 0;
	};
}


