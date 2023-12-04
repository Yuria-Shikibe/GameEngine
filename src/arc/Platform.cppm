module ;

#if defined(_WIN64) || defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#endif

export module Platform;

import <string>;
import File;

export namespace ext{
	OS::File getSelf_runTime(){
		std::string pathName;
#if defined(_WIN64) || defined(_WIN32)
		char absolutePath[MAX_PATH] = {};
		GetModuleFileNameA(nullptr, absolutePath, MAX_PATH);
		pathName = absolutePath;
#elif defined(__linux__)
		char result[PATH_MAX];
	    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	    pathName = std::string(result, (count > 0) ? count : 0);
#endif
		return OS::File{pathName};
	}
}
