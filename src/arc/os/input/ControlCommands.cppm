//
// Created by Matrix on 2023/12/2.
//

module;

export module Ctrl.ControlCommands;

import Ctrl.Constants;

import Core.Input;
import OS;
import OS.Key;
import OS.Mouse;
import Graphic;
import Core;
import <array>;
import <iostream>;

using namespace Core;

static float baseMoveSpeed = 60;
//TODO finish this shit
export namespace Ctrl{
	void registerCommands(Input* const input) { // NOLINT(*-non-const-parameter)
		input->scrollListener.emplace_back([]([[maybe_unused]] float x, const float y) -> void {
			camera->setScale(camera->getScale() + y * OS::delta() * 5.0f);
		});

		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS, []{baseMoveSpeed = 200;}));
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE, []{baseMoveSpeed =  60;}));

		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_A, Act_Continuous, []{camera->trans(-baseMoveSpeed * OS::delta(), 0);}));
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_D, Act_Continuous, []{camera->trans( baseMoveSpeed * OS::delta(), 0);}));
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_W, Act_Continuous, []{camera->trans(0,  baseMoveSpeed * OS::delta());}));
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_S, Act_Continuous, [] {
			camera->trans(0, -baseMoveSpeed * OS::delta());
		}));

		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_F11, GLFW_PRESS, [] {
			if(currentMonitor == mainMonitor) {
				Graphic::windowize(window, currentMonitor, lastScreenBound, title);
				renderer->resize(lastScreenBound.getWidth(), lastScreenBound.getHeight());
			}else {
				Graphic::fullScreen(window, currentMonitor, title);
				renderer->resize(mainScreenMode->width, mainScreenMode->height);
			}
		}));

		auto keys = std::array{OS::KeyBind(GLFW_KEY_LEFT_SHIFT, Act_Continuous), OS::KeyBind(GLFW_KEY_SPACE, GLFW_PRESS) };
		input->registerKeyBindMulti(keys, []() {camera->setScale(1.0f);});
	}
}
