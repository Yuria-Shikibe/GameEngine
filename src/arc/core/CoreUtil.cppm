//
// Created by Matrix on 2024/5/10.
//

export module Core:Util;

import Geom.Vector2D;
import Geom.Rect_Orthogonal;

import :Global;

export namespace Core::Util{
	[[nodiscard]] Geom::Vec2 getMouseToWorld(){
		return Core::camera->getScreenToWorld() * Core::renderer->getNormalized(Core::input.getCursorPos());
	}

	[[nodiscard]] constexpr Geom::Vec2 getNormalized(Geom::Vec2 vec2, const Geom::OrthoRectFloat bound){
		return (vec2.sub(bound.getSrc()) / bound.getSize()).sub(0.5f, 0.5f).scl(2);
	}

	[[nodiscard]] Geom::Vec2 getMouseToWorld(const Core::Camera2D& camera2D, const Geom::OrthoRectFloat bound){
		return camera2D.getScreenToWorld() * getNormalized(Core::input.getCursorPos(), bound);
	}

	void activateMainKeyInput() noexcept{
		Core::input.binds.activate();
	}

	void deactivateMainKeyInput() noexcept{
		Core::input.binds.deactivate();
	}

	void activeBinds(const OS::InputBindGroup* binds){
		Core::input.activeBinds(binds);
	}

	void deactiveBinds(const OS::InputBindGroup* binds){
		Core::input.deactiveBinds(binds);
	}
}
