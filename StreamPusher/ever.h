#pragma once

//#define STATICX

#ifndef STATICX
#if defined(_WIN32) && defined(RTMPPUSHER_EXPORTS)
#ifndef EXPORT_DLL
#define EXPORT_DLL __declspec(dllexport)
#endif //EXPORT_DLL
#ifndef IMPORT_DLL
#define IMPORT_DLL __declspec(dllimport)
#endif  //IMPORT_DLL

#ifndef EXPORT_DLLA
#define EXPORT_DLLA extern"C" __declspec(dllexport)
#endif //EXPORT_DLLA
#ifndef IMPORT_DLLA
#define IMPORT_DLLA extern"C" __declspec(dllimport)
#endif //IMPORT_DLLA
#else
#ifndef EXPORT_DLL
#define EXPORT_DLL
#endif //EXPORT_DLL
#ifndef IMPORT_DLL
#define IMPORT_DLL
#endif //
#ifndef EXPORT_DLLA
#define EXPORT_DLLA
#endif 
#ifndef IMPORT_DLLA
#define IMPORT_DLLA
#endif //IMPORT_DLLA
#endif
#else
#ifdef _WIN32
#ifndef EXPORT_DLLA
#define EXPORT_DLLA extern"C" __declspec(dllexport)
#endif

#ifdef IMPORT_DLLA
#define IMPORT_DLLA extern"C" __declspec(dllimport)
#endif
#else
#ifndef EXPORT_DLLA
#define EXPORT_DLLA
#endif

#ifdef IMPORT_DLLA
#define IMPORT_DLLA
#endif //IMPORT_DLLA
#endif
#endif