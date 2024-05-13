//
// Created by Matrix on 2024/5/10.
//

export module OS.Ctrl:CameraControl;

export import Core.Camera;
export import Geom.Vector2D;
import :FocusInterface;
import :Guard;

export namespace Ctrl{
	struct CameraControl2D : FocusInterface<Core::Camera2D*>::Type<>{
		Core::Camera2D* current{nullptr};
		using Guard = Guard<CameraControl2D, &CameraControl2D::current>;

		void switchFocus(Core::Camera2D* focus){
			this->current = focus;
		}

		void move(const Geom::Vec2 movement){
			check();
			current->move(movement);
		}

		auto operator->() const{
			return current;
		}
	};
}
