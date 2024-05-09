//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_SECURE_TRANSPORT_UTIL_H
#define ICE_SSL_SECURE_TRANSPORT_UTIL_H

#ifdef __APPLE__

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
    CFArrayRef loadCertificateChain(
        const std::string&,
        const std::string&,
        const std::string&,
        const std::string&,
        const std::string&);

    SecCertificateRef loadCertificate(const std::string&);
    CFArrayRef loadCACertificates(const std::string&);
    CFArrayRef findCertificateChain(const std::string&, const std::string&, const std::string&);

}
#endif

#endif
