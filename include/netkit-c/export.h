#ifndef NETKIT_EXPORT_H
#define NETKIT_EXPORT_H

#if defined(_WIN32) && defined(NETKIT_C_SHARED)
#ifdef NETKIT_C_BUILD_DLL
#define NETKIT_C_API __declspec(dllexport)
#else
#define NETKIT_C_API __declspec(dllimport)
#endif
#else
#define NETKIT_C_API __attribute__((visibility("default")))
#endif

#endif