
#pragma once

#if defined(_WIN32) || defined(_WIN64)

#define API_EXPORT __declspec(dllexport)
#define API_IMPORT __declspec(dllimport)
// #define API_SPEC

#else
#error Unsupported Platform

#endif





