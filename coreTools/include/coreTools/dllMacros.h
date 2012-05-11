#ifndef GW2RE_CORETOOLS_DLLMACROS_H
#define GW2RE_CORETOOLS_DLLMACROS_H

#ifdef GW2RE_CORETOOLS_EXPORTS
#define GW2RE_CORETOOLS_API __declspec(dllexport)
#else
#define GW2RE_CORETOOLS_API __declspec(dllimport)
#endif

#endif // GW2RE_CORETOOLS_DLLMACROS_H
