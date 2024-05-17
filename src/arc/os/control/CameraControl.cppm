//
// Created by Matrix on 2024/5/10.
//

export module OS.Ctrl:CameraControl;

export import Core.Camera;
export import Geom.Vector2D;
import :FocusInterface;
import ext.Guard;

export namespace Ctrl{
	struct CameraControl2D : FocusData<Core::Camera2D*>{
		Core::Camera2D* current{nullptr};
		using Guard = ext::Guard<CameraControl2D, &CameraControl2D::current>;

		void switchFocus(Core::Camera2D* focus){
			this->current = focus;
		}

		void move(const Geom::Vec2 movement) const{
			if(current)current->move(movement);
		}

		void set(const Geom::Vec2 pos) const{
			if(current)current->setPosition(pos);
		}

		auto operator->() const{
			return current;
		}
	};
}
