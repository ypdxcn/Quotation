#ifndef _COMM_AP_SAMPLER_H
#define _COMM_AP_SAMPLER_H
#include "CommAp.h"

#ifdef _WIN32
#ifdef COMMAPSAMPLER_EXPORT
#define COMMAPSAMPLER_API			__declspec(dllexport)
#define COMMAPSAMPLER_CLASS		__declspec(dllexport)
#else
#define COMMAPSAMPLER_API			__declspec(dllimport)
#define COMMAPSAMPLER_CLASS		__declspec(dllimport)
#endif
#else
#define COMMAPSAMPLER_API
#define COMMAPSAMPLER_CLASS
#endif

#endif