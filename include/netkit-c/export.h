#ifndef NETKIT_EXPORT_H
#define NETKIT_EXPORT_H

#ifdef _WIN32
#ifdef NETKIT_C_BUILD_DLL
#define NETKIT_C_API __declspec(dllexport)
#else
#define NETKIT_C_API __declspec(dllimport)
#endif
#else
#define NETKIT_C_API
#endif

#endif