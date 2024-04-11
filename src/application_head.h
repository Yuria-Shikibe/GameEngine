// ReSharper disable CppNonInlineVariableDefinitionInHeaderFile
#ifndef APPLICATION_HEAD_H
#define APPLICATION_HEAD_H
	extern "C" {
		__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	}


	extern "C" {
		_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	}
#endif //APPLICATION_HEAD_H