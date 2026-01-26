/** netkit
 *  C++23 cross-platform networking toolkit library providing safe Unix-style sockets and protocol abstractions.
 *
 *  Copyright (c) 2025-2026 Jacob Nilsson
 *  Licensed under the MIT License.
 *
 *  @file certs.cpp
 *  @license MIT
 *  @note Part of the Netkit library.
 *  @note Never include this file from external code; it is for internal use only.
 *  @brief Implementation of functions for exporting Windows system certificates for OpenSSL use.
 */
#include <netkit/crypto/windows/certs.hpp>
#pragma comment(lib, "crypt32.lib")

#ifdef NETKIT_WINDOWS
#ifdef NETKIT_OPENSSL

#include <openssl/ssl.h>

bool netkit::crypto::windows::is_outdated(const std::wstring &path) {
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (!GetFileAttributesExW(path.c_str(),
                              GetFileExInfoStandard,
                              &data))
        return true;

    FILETIME now;
    GetSystemTimeAsFileTime(&now);

    ULARGE_INTEGER file_time, now_time;
    file_time.LowPart = data.ftLastWriteTime.dwLowDateTime;
    file_time.HighPart = data.ftLastWriteTime.dwHighDateTime;

    now_time.LowPart = now.dwLowDateTime;
    now_time.HighPart = now.dwHighDateTime;

    ULONGLONG age = (now_time.QuadPart - file_time.QuadPart) / 10000000ULL;

    constexpr ULONGLONG max_age = 30ULL * 24 * 3600;

    return age > max_age;
}

bool netkit::crypto::windows::export_certs(const std::wstring &path) {
    FILE* f = _wfopen(path.c_str(), L"wb");
    if (!f) return false;

    auto export_store = [&](HCERTSTORE store) {
        PCCERT_CONTEXT cert = nullptr;
        while ((cert = CertEnumCertificatesInStore(store, cert))) {
            DWORD size = 0;
            CryptBinaryToStringA(
                cert->pbCertEncoded,
                cert->cbCertEncoded,
                CRYPT_STRING_BASE64HEADER,
                nullptr,
                &size);

            std::string pem(size, '\0');
            CryptBinaryToStringA(
                cert->pbCertEncoded,
                cert->cbCertEncoded,
                CRYPT_STRING_BASE64HEADER,
                pem.data(),
                &size);

            fwrite(pem.data(), 1, size, f);
        }
    };

    HCERTSTORE root_store = CertOpenSystemStoreW(reinterpret_cast<HCRYPTPROV_LEGACY>(nullptr), L"ROOT");
    if (root_store) {
        export_store(root_store);
        CertCloseStore(root_store, 0);
    }

    HCERTSTORE ca_store = CertOpenSystemStoreW(reinterpret_cast<HCRYPTPROV_LEGACY>(nullptr), L"CA");
    if (ca_store) {
        export_store(ca_store);
        CertCloseStore(ca_store, 0);
    }

    fclose(f);
    return true;
}

#endif
#endif