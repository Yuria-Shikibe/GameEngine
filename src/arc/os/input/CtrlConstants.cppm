//
// Created by Matrix on 2023/12/2.
//

module;

export module Ctrl.Constants;

import GL.Constants;
import Math;
import std;

export namespace Ctrl {
	constexpr int Act_Press = GLFW_PRESS;
	constexpr int Act_Release = GLFW_RELEASE;
	constexpr int Act_Repeat = GLFW_REPEAT;
	constexpr int Act_Continuous = 10;
	constexpr int Act_DoubleClick = 11;

	constexpr int Mode_Shift = 0b0000'0001;
	constexpr int Mode_Ctrl = 0b0000'0010;
	constexpr int Mode_Ctrl_Shift = Mode_Ctrl | Mode_Shift;
	constexpr int Mode_Alt = 0b0000'0100;
	constexpr int Mode_Win = 0b0000'1000;
	constexpr int Mode_NO_IGNORE = 0xff'00'00'00;
	constexpr int Mode_IGNORE = 0xf0'00'00'00;
	constexpr std::array Modes{
		Mode_Shift, Mode_Ctrl, Mode_Alt, Mode_Win
	};

	constexpr float multiPressMaxSpaceing = 0.25f * 60; //ticks!
	constexpr float doublePressMaxSpaceing = 0.25f * 60; //ticks!

	inline bool isContinuous(const int mode) {
		return mode == Ctrl::Act_Continuous;
	}

	// constexpr std::array allModes{
	// 	[]() constexpr {
	// 		std::array<>
	// 		int mode{0};
	// 		for(int i = 0; i < Math::powIntegral<Modes.size()>(2); ++i){
	// 			for(int j = 0; j < Modes.size(); ++j){
	//
	// 			}
	// 		}
	// 	}();
	// }

	constexpr auto HAT_CENTERED = 0;
	constexpr auto HAT_UP = 1;
	constexpr auto HAT_RIGHT = 2;
	constexpr auto HAT_DOWN = 4;
	constexpr auto HAT_LEFT = 8;
	constexpr auto HAT_RIGHT_UP = GLFW_HAT_UP;
	constexpr auto HAT_RIGHT_DOWN = GLFW_HAT_DOWN;
	constexpr auto HAT_LEFT_UP = GLFW_HAT_UP;
	constexpr auto HAT_LEFT_DOWN = GLFW_HAT_DOWN;
	constexpr auto KEY_UNKNOWN = -1;
	constexpr auto KEY_SPACE = 32;
	constexpr auto KEY_APOSTROPHE =        39;  /* ' */
	constexpr auto KEY_COMMA =             44;  /* , */
	constexpr auto KEY_MINUS =             45;  /* - */
	constexpr auto KEY_PERIOD =            46;  /* . */
	constexpr auto KEY_SLASH =             47;  /* / */
	constexpr auto KEY_0 = 48;
	constexpr auto KEY_1 = 49;
	constexpr auto KEY_2 = 50;
	constexpr auto KEY_3 = 51;
	constexpr auto KEY_4 = 52;
	constexpr auto KEY_5 = 53;
	constexpr auto KEY_6 = 54;
	constexpr auto KEY_7 = 55;
	constexpr auto KEY_8 = 56;
	constexpr auto KEY_9 = 57;
	constexpr auto KEY_SEMICOLON         = 59; /* ; */
	constexpr auto KEY_EQUAL             = 61; /* = */
	constexpr auto KEY_A = 65;
	constexpr auto KEY_B = 66;
	constexpr auto KEY_C = 67;
	constexpr auto KEY_D = 68;
	constexpr auto KEY_E = 69;
	constexpr auto KEY_F = 70;
	constexpr auto KEY_G = 71;
	constexpr auto KEY_H = 72;
	constexpr auto KEY_I = 73;
	constexpr auto KEY_J = 74;
	constexpr auto KEY_K = 75;
	constexpr auto KEY_L = 76;
	constexpr auto KEY_M = 77;
	constexpr auto KEY_N = 78;
	constexpr auto KEY_O = 79;
	constexpr auto KEY_P = 80;
	constexpr auto KEY_Q = 81;
	constexpr auto KEY_R = 82;
	constexpr auto KEY_S = 83;
	constexpr auto KEY_T = 84;
	constexpr auto KEY_U = 85;
	constexpr auto KEY_V = 86;
	constexpr auto KEY_W = 87;
	constexpr auto KEY_X = 88;
	constexpr auto KEY_Y = 89;
	constexpr auto KEY_Z = 90;
	constexpr auto KEY_LEFT_BRACKET      = 91  ;/* [ */
	constexpr auto KEY_BACKSLASH         = 92  ;/* \ */
	constexpr auto KEY_RIGHT_BRACKET     = 93  ;/* ] */
	constexpr auto KEY_GRAVE_ACCENT      = 96  ;/* ` */
	constexpr auto KEY_WORLD_1           = 161 ;/* non-US #1 */
	constexpr auto KEY_WORLD_2           = 162 ;/* non-US #2 */
	constexpr auto KEY_ESCAPE = 256;
	constexpr auto KEY_ENTER = 257;
	constexpr auto KEY_TAB = 258;
	constexpr auto KEY_BACKSPACE = 259;
	constexpr auto KEY_INSERT = 260;
	constexpr auto KEY_DELETE = 261;
	constexpr auto KEY_RIGHT = 262;
	constexpr auto KEY_LEFT = 263;
	constexpr auto KEY_DOWN = 264;
	constexpr auto KEY_UP = 265;
	constexpr auto KEY_PAGE_UP = 266;
	constexpr auto KEY_PAGE_DOWN = 267;
	constexpr auto KEY_HOME = 268;
	constexpr auto KEY_END = 269;
	constexpr auto KEY_CAPS_LOCK = 280;
	constexpr auto KEY_SCROLL_LOCK = 281;
	constexpr auto KEY_NUM_LOCK = 282;
	constexpr auto KEY_PRINT_SCREEN = 283;
	constexpr auto KEY_PAUSE = 284;
	constexpr auto KEY_F1 = 290;
	constexpr auto KEY_F2 = 291;
	constexpr auto KEY_F3 = 292;
	constexpr auto KEY_F4 = 293;
	constexpr auto KEY_F5 = 294;
	constexpr auto KEY_F6 = 295;
	constexpr auto KEY_F7 = 296;
	constexpr auto KEY_F8 = 297;
	constexpr auto KEY_F9 = 298;
	constexpr auto KEY_F10 = 299;
	constexpr auto KEY_F11 = 300;
	constexpr auto KEY_F12 = 301;
	constexpr auto KEY_F13 = 302;
	constexpr auto KEY_F14 = 303;
	constexpr auto KEY_F15 = 304;
	constexpr auto KEY_F16 = 305;
	constexpr auto KEY_F17 = 306;
	constexpr auto KEY_F18 = 307;
	constexpr auto KEY_F19 = 308;
	constexpr auto KEY_F20 = 309;
	constexpr auto KEY_F21 = 310;
	constexpr auto KEY_F22 = 311;
	constexpr auto KEY_F23 = 312;
	constexpr auto KEY_F24 = 313;
	constexpr auto KEY_F25 = 314;
	constexpr auto KEY_KP_0 = 320;
	constexpr auto KEY_KP_1 = 321;
	constexpr auto KEY_KP_2 = 322;
	constexpr auto KEY_KP_3 = 323;
	constexpr auto KEY_KP_4 = 324;
	constexpr auto KEY_KP_5 = 325;
	constexpr auto KEY_KP_6 = 326;
	constexpr auto KEY_KP_7 = 327;
	constexpr auto KEY_KP_8 = 328;
	constexpr auto KEY_KP_9 = 329;
	constexpr auto KEY_KP_DECIMAL = 330;
	constexpr auto KEY_KP_DIVIDE = 331;
	constexpr auto KEY_KP_MULTIPLY = 332;
	constexpr auto KEY_KP_SUBTRACT = 333;
	constexpr auto KEY_KP_ADD = 334;
	constexpr auto KEY_KP_ENTER = 335;
	constexpr auto KEY_KP_EQUAL = 336;
	constexpr auto KEY_LEFT_SHIFT = 340;
	constexpr auto KEY_LEFT_CONTROL = 341;
	constexpr auto KEY_LEFT_ALT = 342;
	constexpr auto KEY_LEFT_SUPER = 343;
	constexpr auto KEY_RIGHT_SHIFT = 344;
	constexpr auto KEY_RIGHT_CONTROL = 345;
	constexpr auto KEY_RIGHT_ALT = 346;
	constexpr auto KEY_RIGHT_SUPER = 347;
	constexpr auto KEY_MENU = 348;
	constexpr auto KEY_LAST = GLFW_KEY_MENU;
	constexpr auto KEY_COUNT = KEY_LAST + 1;

	constexpr auto MOUSE_BUTTON_1 = 0;
	constexpr auto MOUSE_BUTTON_2 = 1;
	constexpr auto MOUSE_BUTTON_3 = 2;
	constexpr auto MOUSE_BUTTON_4 = 3;
	constexpr auto MOUSE_BUTTON_5 = 4;
	constexpr auto MOUSE_BUTTON_6 = 5;
	constexpr auto MOUSE_BUTTON_7 = 6;
	constexpr auto MOUSE_BUTTON_8 = 7;
	constexpr auto MOUSE_BUTTON_LAST = MOUSE_BUTTON_8;
	constexpr auto MOUSE_BUTTON_COUNT = MOUSE_BUTTON_LAST + 1;
	constexpr auto MOUSE_BUTTON_LEFT = MOUSE_BUTTON_1;
	constexpr auto MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_2;
	constexpr auto MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3;

	constexpr auto LMB = MOUSE_BUTTON_1;
	constexpr auto RMB = MOUSE_BUTTON_2;
	constexpr auto CMB = MOUSE_BUTTON_3;
}
