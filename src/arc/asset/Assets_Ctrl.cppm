//
// Created by Matrix on 2024/5/10.
//

export module Assets.Ctrl;

export import Core;
export import OS.Ctrl.Bind.Constants;
export import OS.Ctrl.Operation;
export import OS.Ctrl.Bind;
export import OS;

namespace Assets::Ctrl{
	using namespace ::Ctrl;
	using namespace ::Core;
}

export namespace Assets::Ctrl{
	inline float baseCameraMoveSpeed = 60;
	inline float fastCameraMoveSpeed = 300;
	inline float cameraMoveSpeed = baseCameraMoveSpeed;

	Operation cameraMoveLeft{"cmove-left", OS::KeyBind(Key::A, Act::Continuous, +[]{
		Core::focus.camera->move(-cameraMoveSpeed * OS::delta(), 0);
	})};

	Operation cameraMoveRight{"cmove-right", OS::KeyBind(Key::D, Act::Continuous, +[]{
		Core::focus.camera->move( cameraMoveSpeed * OS::delta(), 0);
	})};

	Operation cameraMoveUp{"cmove-up", OS::KeyBind(Key::W, Act::Continuous, +[]{
		Core::focus.camera->move(0,  cameraMoveSpeed * OS::delta());
	})};

	Operation cameraMoveDown{"cmove-down", OS::KeyBind(Key::S, Act::Continuous, +[] {
		Core::focus.camera->move(0, -cameraMoveSpeed * OS::delta());
	})};

	Operation boostCamera_Press{"cmrboost-prs", OS::KeyBind(Key::Shift_Left, Act::Press, +[] {
		cameraMoveSpeed = fastCameraMoveSpeed;
	})};

	Operation boostCamera_Release{"cmrboost-prs", OS::KeyBind(Key::Shift_Left, Act::Release, +[] {
		cameraMoveSpeed = baseCameraMoveSpeed;
	})};
}
