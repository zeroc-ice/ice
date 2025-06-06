// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_SSL_UTIL_H
#define ICE_SSL_SSL_UTIL_H

#include "Ice/Config.h"
#include "Ice/SSL/Config.h"

#include <optional>
#include <string>
#include <vector>

namespace Ice::SSL
{
#if defined(__APPLE__)
    //
    // Helper functions to use by Secure Transport.
    //
    std::string fromCFString(CFStringRef);

    inline CFStringRef toCFString(const std::string& s)
    {
        return CFStringCreateWithCString(nullptr, s.c_str(), kCFStringEncodingUTF8);
    }
#endif

    // Constants for X509 certificate alt names (AltNameOther, AltNameORAddress, AltNameEDIPartyName and
    // AltNameObjectIdentifier) are not supported.

    // const int AltNameOther = 0;
    const int AltNameEmail = 1;
    const int AltNameDNS = 2;
    // const int AltNameORAddress = 3;
    const int AltNameDirectory = 4;
    // const int AltNameEDIPartyName = 5;
    const int AltNameURL = 6;
    const int AltNAmeIP = 7;
    // const AltNameObjectIdentifier = 8;

    // Read a file into memory buffer.
    void readFile(const std::string&, std::vector<char>&);

    // Determine if a file exists, with an optional parent directory.
    std::optional<std::string> resolveFilePath(const std::string& path, const std::string& parentDir = "");

    // Determine if a directory exists, with an optional parent directory.
    std::optional<std::string> resolveDirPath(const std::string& path, const std::string& parentDir = "");

    bool parseBytes(const std::string&, std::vector<unsigned char>&);

#if defined(ICE_USE_SCHANNEL)
    ICE_API std::string getSubjectName(PCCERT_CONTEXT);
    std::vector<std::pair<int, std::string>> getSubjectAltNames(PCCERT_CONTEXT);
    ICE_API std::string encodeCertificate(PCCERT_CONTEXT);

    class ICE_API ScopedCertificate
    {
    public:
        ScopedCertificate(PCCERT_CONTEXT certificate) : _certificate(certificate) {}
        ~ScopedCertificate();
        PCCERT_CONTEXT get() const { return _certificate; }

    private:
        PCCERT_CONTEXT _certificate;
    };
    ICE_API PCCERT_CONTEXT decodeCertificate(const std::string&);
#elif defined(ICE_USE_SECURE_TRANSPORT)
    std::string certificateOIDAlias(const std::string&);
    ICE_API std::string getSubjectName(SecCertificateRef);
    std::vector<std::pair<int, std::string>> getSubjectAltNames(SecCertificateRef);
    ICE_API std::string encodeCertificate(SecCertificateRef);

    class ICE_API ScopedCertificate
    {
    public:
        ScopedCertificate(SecCertificateRef certificate) : _certificate(certificate) {}
        ~ScopedCertificate();
        [[nodiscard]] SecCertificateRef get() const { return _certificate; }

    private:
        SecCertificateRef _certificate;
    };
    ICE_API SecCertificateRef decodeCertificate(const std::string&);
#elif defined(ICE_USE_OPENSSL)
    // Accumulate the OpenSSL error stack into a string.
    std::string getErrors();
    ICE_API std::string getSubjectName(X509*);
    std::vector<std::pair<int, std::string>> getSubjectAltNames(X509*);
    ICE_API std::string encodeCertificate(X509*);
    class ICE_API ScopedCertificate
    {
    public:
        ScopedCertificate(X509* certificate) : _certificate(certificate) {}
        ~ScopedCertificate();
        [[nodiscard]] X509* get() const { return _certificate; }

    private:
        X509* _certificate;
    };
    ICE_API X509* decodeCertificate(const std::string&);
#endif
}

#endif
