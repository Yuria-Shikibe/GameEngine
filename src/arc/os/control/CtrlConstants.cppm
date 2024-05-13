module;

#include <GLFW/glfw3.h>

export module OS.Ctrl.Bind.Constants;

import std;

export namespace Ctrl {
	namespace Act{
		constexpr int Press = GLFW_PRESS;
		constexpr int Release = GLFW_RELEASE;
		constexpr int Repeat = GLFW_REPEAT;
		constexpr int Continuous = 3;
		constexpr int DoubleClick = 4;
	}
	
	namespace Mode{
		constexpr int None = 0;
		constexpr int Shift = GLFW_MOD_SHIFT;
		constexpr int Ctrl = GLFW_MOD_CONTROL;
		constexpr int Alt = GLFW_MOD_ALT;
		constexpr int Super = GLFW_MOD_SUPER;

		constexpr int CapLock = GLFW_MOD_CAPS_LOCK;
		constexpr int NumLock = GLFW_MOD_NUM_LOCK;
		//6Bit

		constexpr int Ctrl_Shift = Ctrl | Shift;

		constexpr int NoIgnore = 0xff'00'00'00;
		constexpr int Ignore = 0xf0'00'00'00;
		constexpr int Disable = -1;

		constexpr int Mask = 0xff;

		constexpr int Frequent_Bound = Shift | Ctrl | Alt/* | Super | CapLock | NumLock */+ 1;

		[[nodiscard]] constexpr bool isDisabled(const int val) noexcept{
			return val == Disable;
		}

		constexpr void setDisabled(int& val) noexcept{
			val = Disable;
		}

		[[nodiscard]] constexpr bool modeMatch(const int mode, const int expectedMode) noexcept{
			return (mode & Mask) == (expectedMode & Mask);
		}
	}


	/**
	 * @code
	 *	0b 0000'0000  0000'0000  0000'0000  0000'0000
	 *	   [  Mode ]  [  Act  ]  [     Key Code     ]
	 * @endcode
	 */
	constexpr int getFullKey(const int keyCode, const int act, const int mode) noexcept{
		return
			keyCode
			| act << 16
			| mode << 8;
	}


	constexpr float multiPressMaxSpaceing = 0.25f * 60; //ticks!
	constexpr float doublePressMaxSpaceing = 0.25f * 60; //ticks!

	constexpr bool isContinuous(const int mode) noexcept{
		return mode == Act::Continuous;
	}

	//Key Total Count -- 350 -- 9bit
	constexpr auto HAT_CENTERED = 0;
	constexpr auto HAT_UP = 1;
	constexpr auto HAT_RIGHT = 2;
	constexpr auto HAT_DOWN = 4;
	constexpr auto HAT_LEFT = 8;
	constexpr auto HAT_RIGHT_UP = GLFW_HAT_UP;
	constexpr auto HAT_RIGHT_DOWN = GLFW_HAT_DOWN;
	constexpr auto HAT_LEFT_UP = GLFW_HAT_UP;
	constexpr auto HAT_LEFT_DOWN = GLFW_HAT_DOWN;

	
	constexpr auto Unknown = -1;

	namespace Key{
		constexpr auto Space = 32;
		constexpr auto Apostrophe =        39;  /* ' */
		constexpr auto Comma =             44;  /* , */
		constexpr auto Minus =             45;  /* - */
		constexpr auto Period =            46;  /* . */
		constexpr auto Slash =             47;  /* / */
		constexpr auto _0 = 48;
		constexpr auto _1 = 49;
		constexpr auto _2 = 50;
		constexpr auto _3 = 51;
		constexpr auto _4 = 52;
		constexpr auto _5 = 53;
		constexpr auto _6 = 54;
		constexpr auto _7 = 55;
		constexpr auto _8 = 56;
		constexpr auto _9 = 57;
		constexpr auto Semicolon         = 59; /* ; */
		constexpr auto Equal             = 61; /* = */
		constexpr auto A = 65;
		constexpr auto B = 66;
		constexpr auto C = 67;
		constexpr auto D = 68;
		constexpr auto E = 69;
		constexpr auto F = 70;
		constexpr auto G = 71;
		constexpr auto H = 72;
		constexpr auto I = 73;
		constexpr auto J = 74;
		constexpr auto K = 75;
		constexpr auto L = 76;
		constexpr auto M = 77;
		constexpr auto N = 78;
		constexpr auto O = 79;
		constexpr auto P = 80;
		constexpr auto Q = 81;
		constexpr auto R = 82;
		constexpr auto S = 83;
		constexpr auto T = 84;
		constexpr auto U = 85;
		constexpr auto V = 86;
		constexpr auto W = 87;
		constexpr auto X = 88;
		constexpr auto Y = 89;
		constexpr auto Z = 90;
		constexpr auto LeftBracket      = 91  ;/* [ */
		constexpr auto Backslash         = 92  ;/* \ */
		constexpr auto RightBracket     = 93  ;/* ] */
		constexpr auto GraveAccent      = 96  ;/* ` */
		constexpr auto WORLD_1           = 161 ;/* non-US #1 */
		constexpr auto WORLD_2           = 162 ;/* non-US #2 */
		constexpr auto Esc = 256;
		constexpr auto Enter = 257;
		constexpr auto Tab = 258;
		constexpr auto Backspace = 259;
		constexpr auto Insert = 260;
		constexpr auto Delete = 261;
		constexpr auto Right = 262;
		constexpr auto Left = 263;
		constexpr auto Down = 264;
		constexpr auto Up = 265;
		constexpr auto PageUp = 266;
		constexpr auto PageDown = 267;
		constexpr auto Home = 268;
		constexpr auto End = 269;
		constexpr auto CapsLock = 280;
		constexpr auto ScrollLock = 281;
		constexpr auto NumLock = 282;
		constexpr auto PrintScreen = 283;
		constexpr auto Pause = 284;
		constexpr auto F1 = 290;
		constexpr auto F2 = 291;
		constexpr auto F3 = 292;
		constexpr auto F4 = 293;
		constexpr auto F5 = 294;
		constexpr auto F6 = 295;
		constexpr auto F7 = 296;
		constexpr auto F8 = 297;
		constexpr auto F9 = 298;
		constexpr auto F10 = 299;
		constexpr auto F11 = 300;
		constexpr auto F12 = 301;
		constexpr auto F13 = 302;
		constexpr auto F14 = 303;
		constexpr auto F15 = 304;
		constexpr auto F16 = 305;
		constexpr auto F17 = 306;
		constexpr auto F18 = 307;
		constexpr auto F19 = 308;
		constexpr auto F20 = 309;
		constexpr auto F21 = 310;
		constexpr auto F22 = 311;
		constexpr auto F23 = 312;
		constexpr auto F24 = 313;
		constexpr auto F25 = 314;
		constexpr auto KP_0 = 320;
		constexpr auto KP_1 = 321;
		constexpr auto KP_2 = 322;
		constexpr auto KP_3 = 323;
		constexpr auto KP_4 = 324;
		constexpr auto KP_5 = 325;
		constexpr auto KP_6 = 326;
		constexpr auto KP_7 = 327;
		constexpr auto KP_8 = 328;
		constexpr auto KP_9 = 329;
		constexpr auto KP_DECIMAL = 330;
		constexpr auto KP_DIVIDE = 331;
		constexpr auto KP_MULTIPLY = 332;
		constexpr auto KP_SUBTRACT = 333;
		constexpr auto KP_ADD = 334;
		constexpr auto KP_ENTER = 335;
		constexpr auto KP_EQUAL = 336;
		constexpr auto Shift_Left = 340;
		constexpr auto LEFT_CONTROL = 341;
		constexpr auto LEFT_ALT = 342;
		constexpr auto LEFT_SUPER = 343;
		constexpr auto Shift_Right = 344;
		constexpr auto RIGHT_CONTROL = 345;
		constexpr auto RIGHT_ALT = 346;
		constexpr auto RIGHT_SUPER = 347;
		constexpr auto MENU = 348;
		constexpr auto LAST = GLFW_KEY_MENU;
		constexpr auto Count = LAST + 1;
	}

	namespace Mouse{
		constexpr auto _1 = 0;
		constexpr auto _2 = 1;
		constexpr auto _3 = 2;
		constexpr auto _4 = 3;
		constexpr auto _5 = 4;
		constexpr auto _6 = 5;
		constexpr auto _7 = 6;
		constexpr auto _8 = 7;
		constexpr auto LAST = 8;
		constexpr auto Count = LAST + 1;
		constexpr auto LEFT = _1;
		constexpr auto RIGHT = _2;
		constexpr auto MIDDLE = _3;

		constexpr auto LMB = _1;
		constexpr auto RMB = _2;
		constexpr auto CMB = _3;
	}

	constexpr auto AllKeyCount = Key::Count;

	constexpr bool onMouse(const int keyCode) noexcept{
		return keyCode < Mouse::Count;
	}

	/**
	 * @brief Caution: Cannot confirm if it is really on the keyboard
	 */
	constexpr bool onKeyboard(const int keyCode) noexcept{
		return !onMouse(keyCode) && keyCode < Key::Count;
	}
}
