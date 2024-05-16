module;

#include <GLFW/glfw3.h>

export module OS.Ctrl.Bind.Constants;

import std;

export namespace Ctrl{
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
		constexpr int Mask = 0b00'11'11'11;

		constexpr int Ctrl_Shift = Ctrl | Shift;

		constexpr int NoIgnore = 0xff'00'00'00;
		constexpr int Ignore = 0xf0'00'00'00;
		constexpr int Disable = -1;

		constexpr int Total = Mask + 1;
		constexpr int TotalBits = 6;

		constexpr int Frequent_Bound = Shift | Ctrl | Alt/* | Super | CapLock | NumLock */ + 1;

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
			| mode << (16 + 8);
	}

	/**
	 * @brief
	 * @return [keyCode, act, mode]
	 */
	constexpr std::tuple<int, int, int> getSeperatedKey(const int fullKey) noexcept{
		return {fullKey & 0xffff, fullKey >> 16 & 0xff, fullKey >> 24 & 0xff};
	}

	constexpr float multiPressMaxSpaceing = 0.25f * 60;  //ticks!
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
		constexpr auto Apostrophe = 39; /* ' */
		constexpr auto Comma = 44;      /* , */
		constexpr auto Minus = 45;      /* - */
		constexpr auto Period = 46;     /* . */
		constexpr auto Slash = 47;      /* / */
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
		constexpr auto Semicolon = 59; /* ; */
		constexpr auto Equal = 61;     /* = */
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
		constexpr auto LeftBracket = 91;  /* [ */
		constexpr auto Backslash = 92;    /* \ */
		constexpr auto RightBracket = 93; /* ] */
		constexpr auto GraveAccent = 96;  /* ` */

		constexpr auto WORLD_1 = 161; /* non-US #1 */
		constexpr auto WORLD_2 = 162; /* non-US #2 */

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

		constexpr auto Left_Shift = 340;
		constexpr auto Left_Control = 341;
		constexpr auto Left_Alt = 342;
		constexpr auto Left_Super = 343;

		constexpr auto Right_Shift = 344;
		constexpr auto Right_Control = 345;
		constexpr auto Right_Alt = 346;
		constexpr auto Right_Super = 347;

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

	constexpr std::array KeyNames{
			[]() constexpr{
				std::array<std::string_view, AllKeyCount> kMap{};

				kMap[0] = "mouse_left";
				kMap[1] = "mouse_right";
				kMap[2] = "mouse_scroll";
				kMap[3] = "mouse_4";
				kMap[4] = "mouse_5";
				kMap[5] = "mouse_6";
				kMap[6] = "mouse_7";
				kMap[7] = "mouse_8";

				kMap[32] = "keyboard_space";
				kMap[39] = "keyboard_apostrophe";
				kMap[48] = "keyboard_0";
				kMap[49] = "keyboard_1";
				kMap[50] = "keyboard_2";
				kMap[51] = "keyboard_3";
				kMap[52] = "keyboard_4";
				kMap[53] = "keyboard_5";
				kMap[54] = "keyboard_6";
				kMap[55] = "keyboard_7";
				kMap[56] = "keyboard_8";
				kMap[57] = "keyboard_9";
				kMap[59] = "keyboard_semicolon";
				kMap[61] = "keyboard_equal";
				kMap[65] = "keyboard_a";
				kMap[66] = "keyboard_b";
				kMap[67] = "keyboard_c";
				kMap[68] = "keyboard_d";
				kMap[69] = "keyboard_e";
				kMap[70] = "keyboard_f";
				kMap[71] = "keyboard_g";
				kMap[72] = "keyboard_h";
				kMap[73] = "keyboard_i";
				kMap[74] = "keyboard_j";
				kMap[75] = "keyboard_k";
				kMap[76] = "keyboard_l";
				kMap[77] = "keyboard_m";
				kMap[78] = "keyboard_n";
				kMap[79] = "keyboard_o";
				kMap[80] = "keyboard_p";
				kMap[81] = "keyboard_q";
				kMap[82] = "keyboard_r";
				kMap[83] = "keyboard_s";
				kMap[84] = "keyboard_t";
				kMap[85] = "keyboard_u";
				kMap[86] = "keyboard_v";
				kMap[87] = "keyboard_w";
				kMap[88] = "keyboard_x";
				kMap[89] = "keyboard_y";
				kMap[90] = "keyboard_z";
				kMap[91] = "keyboard_leftbracket";
				kMap[92] = "keyboard_backslash";
				kMap[93] = "keyboard_rightbracket";
				kMap[96] = "keyboard_graveaccent";
				kMap[161] = "keyboard_world_1";
				kMap[162] = "keyboard_world_2";
				kMap[256] = "keyboard_esc";
				kMap[257] = "keyboard_enter";
				kMap[258] = "keyboard_tab";
				kMap[259] = "keyboard_backspace";
				kMap[260] = "keyboard_insert";
				kMap[261] = "keyboard_delete";
				kMap[262] = "keyboard_right";
				kMap[263] = "keyboard_left";
				kMap[264] = "keyboard_down";
				kMap[265] = "keyboard_up";
				kMap[266] = "keyboard_pageup";
				kMap[267] = "keyboard_pagedown";
				kMap[268] = "keyboard_home";
				kMap[269] = "keyboard_end";
				kMap[280] = "keyboard_capslock";
				kMap[281] = "keyboard_scrolllock";
				kMap[282] = "keyboard_numlock";
				kMap[283] = "keyboard_printscreen";
				kMap[284] = "keyboard_pause";
				kMap[290] = "keyboard_f1";
				kMap[291] = "keyboard_f2";
				kMap[292] = "keyboard_f3";
				kMap[293] = "keyboard_f4";
				kMap[294] = "keyboard_f5";
				kMap[295] = "keyboard_f6";
				kMap[296] = "keyboard_f7";
				kMap[297] = "keyboard_f8";
				kMap[298] = "keyboard_f9";
				kMap[299] = "keyboard_f10";
				kMap[300] = "keyboard_f11";
				kMap[301] = "keyboard_f12";
				kMap[302] = "keyboard_f13";
				kMap[303] = "keyboard_f14";
				kMap[304] = "keyboard_f15";
				kMap[305] = "keyboard_f16";
				kMap[306] = "keyboard_f17";
				kMap[307] = "keyboard_f18";
				kMap[308] = "keyboard_f19";
				kMap[309] = "keyboard_f20";
				kMap[310] = "keyboard_f21";
				kMap[311] = "keyboard_f22";
				kMap[312] = "keyboard_f23";
				kMap[313] = "keyboard_f24";
				kMap[314] = "keyboard_f25";
				kMap[320] = "keyboard_kp_0";
				kMap[321] = "keyboard_kp_1";
				kMap[322] = "keyboard_kp_2";
				kMap[323] = "keyboard_kp_3";
				kMap[324] = "keyboard_kp_4";
				kMap[325] = "keyboard_kp_5";
				kMap[326] = "keyboard_kp_6";
				kMap[327] = "keyboard_kp_7";
				kMap[328] = "keyboard_kp_8";
				kMap[329] = "keyboard_kp_9";
				kMap[330] = "keyboard_kp_decimal";
				kMap[331] = "keyboard_kp_divide";
				kMap[332] = "keyboard_kp_multiply";
				kMap[333] = "keyboard_kp_subtract";
				kMap[334] = "keyboard_kp_add";
				kMap[335] = "keyboard_kp_enter";
				kMap[336] = "keyboard_kp_equal";
				kMap[340] = "keyboard_left_shift";
				kMap[341] = "keyboard_left_control";
				kMap[342] = "keyboard_left_alt";
				kMap[343] = "keyboard_left_super";
				kMap[344] = "keyboard_right_shift";
				kMap[345] = "keyboard_right_control";
				kMap[346] = "keyboard_right_alt";
				kMap[347] = "keyboard_right_super";
				kMap[348] = "keyboard_menu";

				return kMap;
			}()
		};

	constexpr std::array ModeNames{
			[]() constexpr{
				std::array<std::string_view, Mode::TotalBits + 1> names{};

				/*
					constexpr int Shift = GLFW_MOD_SHIFT;
					constexpr int Ctrl = GLFW_MOD_CONTROL;
					constexpr int Alt = GLFW_MOD_ALT;
					constexpr int Super = GLFW_MOD_SUPER;

					constexpr int CapLock = GLFW_MOD_CAPS_LOCK;
					constexpr int NumLock = GLFW_MOD_NUM_LOCK;
				 */
				names[/*0b0000'0000*/ 0] = "none";
				names[/*0b0000'0001*/ 1] = "shift";
				names[/*0b0000'0010*/ 2] = "ctrl";
				names[/*0b0000'0100*/ 3] = "alt";
				names[/*0b0000'1000*/ 4] = "super";
				names[/*0b0001'0000*/ 5] = "cap-lock";
				names[/*0b0010'0000*/ 6] = "num-lock";

				return names;
			}()
		};

	constexpr std::array ActNames{
		[]() constexpr{
			std::array<std::string_view, 5> names{};

			/*
			constexpr int Press = GLFW_PRESS;
			constexpr int Release = GLFW_RELEASE;
			constexpr int Repeat = GLFW_REPEAT;
			constexpr int Continuous = 3;
			constexpr int DoubleClick = 4;
			 */
			names[Act::Press]       = "press";
			names[Act::Release]     = "release";
			names[Act::Repeat]      = "repeat";
			names[Act::DoubleClick] = "double";
			names[Act::Continuous]  = "continuous";

			return names;
		}()
	};

	constexpr std::vector<std::string_view> getModesStr(const int mode, const bool emptyPatch = false){
		std::vector<std::string_view> strs{};

		for(int i = 0; i < Mode::TotalBits; ++i){
			int curMode = 1 << i;
			if(mode & curMode){
				strs.push_back(ModeNames[i + 1]);
				strs.push_back(" + ");
			}
		}


		if(strs.empty()){
			if(emptyPatch)strs.push_back(ModeNames[0]);
		}else{
			strs.pop_back();
		}

		return strs;
	}
}
