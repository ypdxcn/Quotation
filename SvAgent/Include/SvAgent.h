#ifndef _SV_AGENT_H
#define _SV_AGENT_H

#include "CommAp.h"

#ifdef _WIN32
#ifdef SVAGENT_EXPORT
#define SVAGENT_API			__declspec(dllexport)
#define SVAGENT_CLASS		__declspec(dllexport)
#else
#define SVAGENT_API			__declspec(dllimport)
#define SVAGENT_CLASS		__declspec(dllimport)
#endif
#else
#define SVAGENT_API
#define SVAGENT_CLASS
#endif

#endif