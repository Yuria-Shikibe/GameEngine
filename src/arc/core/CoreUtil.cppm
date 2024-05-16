//
// Created by Matrix on 2024/5/10.
//

export module Core:Util;

import Geom.Vector2D;

import :Global;

export namespace Core::Util{
	[[nodiscard]] Geom::Vec2 getMouseToWorld(){
		return Core::camera->getScreenToWorld() * Core::renderer->getNormalized(Core::input.getCursorPos());
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
