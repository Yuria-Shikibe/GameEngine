// ReSharper disable CppNonInlineVariableDefinitionInHeaderFile
#ifndef APPLICATION_HEAD_H
#define APPLICATION_HEAD_H
	#if defined(_WIN64) || defined(_WIN32)
	#include <Windows.h>
	extern "C" {
		__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	}


	extern "C" {
		_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	}
	#endif
#endif //APPLICATION_HEAD_H
