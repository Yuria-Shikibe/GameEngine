//
// Created by Matrix on 2024/5/10.
//

export module Assets.Ctrl;

export import Core;
export import Core.MainLoopManager;
import UI.Screen;
export import OS.Ctrl.Bind.Constants;
export import OS.Ctrl.Operation;
export import OS.Ctrl.Bind;
export import OS;
import std;

import ext.Heterogeneous;

namespace Assets::Ctrl{
	float baseCameraMoveSpeed = 60;
	float fastCameraMoveSpeed = 300;
	bool disableMove = false;
	float cameraMoveSpeed = baseCameraMoveSpeed;


	::Ctrl::Operation cameraMoveLeft{"cmove-left", OS::KeyBind(::Ctrl::Key::Left, ::Ctrl::Act::Continuous, +[]{
		if(!disableMove)Core::focus.camera.move({-cameraMoveSpeed * Core::loopManager->delta(), 0});
	})};

	::Ctrl::Operation cameraMoveRight{"cmove-right", OS::KeyBind(::Ctrl::Key::Right, ::Ctrl::Act::Continuous, +[]{
		if(!disableMove)Core::focus.camera.move({cameraMoveSpeed * Core::loopManager->delta(), 0});
	})};

	::Ctrl::Operation cameraMoveUp{"cmove-up", OS::KeyBind(::Ctrl::Key::Up, ::Ctrl::Act::Continuous, +[]{
		if(!disableMove)Core::focus.camera.move({0,  cameraMoveSpeed * Core::loopManager->delta()});
	})};

	::Ctrl::Operation cameraMoveDown{"cmove-down", OS::KeyBind(::Ctrl::Key::Down, ::Ctrl::Act::Continuous, +[] {
		if(!disableMove)Core::focus.camera.move({0, -cameraMoveSpeed * Core::loopManager->delta()});
	})};

	::Ctrl::Operation cameraTeleport{"cmove-telp", OS::KeyBind(::Ctrl::Mouse::LMB, ::Ctrl::Act::DoubleClick, +[] {
		if(Core::uiRoot->cursorCaptured()){

		}else{
			Core::camera->setPosition(Core::Util::getMouseToWorld());
		}
	})};

	::Ctrl::Operation cameraLock{"cmove-lock", OS::KeyBind(::Ctrl::Key::M, ::Ctrl::Act::Press, +[] {
		disableMove = !disableMove;
	})};

	::Ctrl::Operation boostCamera_Release{"cmrboost-rls", OS::KeyBind(::Ctrl::Key::Left_Shift, ::Ctrl::Act::Release, +[] {
		cameraMoveSpeed = baseCameraMoveSpeed;
	})};

	::Ctrl::Operation boostCamera_Press{"cmrboost-prs", OS::KeyBind(::Ctrl::Key::Left_Shift, ::Ctrl::Act::Press, +[] {
		cameraMoveSpeed = fastCameraMoveSpeed;
	}), {boostCamera_Release.name}};

	::Ctrl::Operation pause{"pause", OS::KeyBind(::Ctrl::Key::P, ::Ctrl::Act::Press, +[] {
		Core::loopManager->timer.setPause(!Core::loopManager->timer.isPaused());
	})};

	::Ctrl::Operation hideUI{"ui-hide", OS::KeyBind(::Ctrl::Key::H, ::Ctrl::Act::Press, +[] {
		if(Core::uiRoot->isHidden){
			Core::uiRoot->show();
		} else{
			if(!Core::uiRoot->hasTextFocus())Core::uiRoot->hide();
		}
	})};

	::Ctrl::Operation shoot{"shoot", OS::KeyBind(::Ctrl::Key::F, ::Ctrl::Act::Continuous, +[] {

	})};

	export{
		OS::InputBindGroup mainControlGroup{};

		::Ctrl::OperationGroup basicGroup{
				"basic-group", {
					::Ctrl::Operation{cameraMoveLeft},
					::Ctrl::Operation{cameraMoveRight},
					::Ctrl::Operation{cameraMoveUp},
					::Ctrl::Operation{cameraMoveDown},
					::Ctrl::Operation{boostCamera_Press},
					::Ctrl::Operation{boostCamera_Release},

					::Ctrl::Operation{cameraLock},
					::Ctrl::Operation{cameraTeleport},

					::Ctrl::Operation{pause},
					::Ctrl::Operation{hideUI},
				}
			};

		::Ctrl::OperationGroup gameGroup{"game-group"};

		ext::StringHashMap<::Ctrl::OperationGroup*> allGroups{
			{basicGroup.getName(), &basicGroup},
			{gameGroup.getName(), &gameGroup}
		};

		ext::StringHashMap<OS::InputBindGroup*> relatives{};

		void apply(){
			mainControlGroup.clearAllBinds();

			for (auto group : allGroups | std::ranges::views::values){
				for (const auto & bind : group->getBinds() | std::ranges::views::values){
					mainControlGroup.registerBind(OS::InputBind{bind.customeBind});
				}
			}
		}

		void load(){
			Core::input.registerSubInput(&mainControlGroup);
			basicGroup.targetGroup = &mainControlGroup;
			apply();
		}
	}
}
