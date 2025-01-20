// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_SECURE_TRANSPORT_UTIL_H
#define ICE_SSL_SECURE_TRANSPORT_UTIL_H

#ifdef __APPLE__

#    include "Ice/Config.h"
#    include "SSLUtil.h"

namespace Ice::SSL::SecureTransport
{
    std::string sslErrorToString(CFErrorRef);
    std::string sslErrorToString(OSStatus);

#    if defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    //
    // Retrieve a certificate property
    //
    CFDictionaryRef getCertificateProperty(SecCertificateRef, CFTypeRef);
#    endif

    //
    // Read certificate from a file.
    //
    ICE_API CFArrayRef loadCertificateChain(
        const std::string&,
        const std::string&,
        const std::string&,
        const std::string&,
        const std::string&);

    SecCertificateRef loadCertificate(const std::string&);
    ICE_API CFArrayRef loadCACertificates(const std::string&);
    ICE_API CFArrayRef findCertificateChain(const std::string&, const std::string&, const std::string&);

}
#endif

#endif
