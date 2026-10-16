// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_APPLE_SSL_UTIL_H
#define ICE_SSL_APPLE_SSL_UTIL_H

#ifdef __APPLE__

#    include "Ice/Config.h"
#    include "SSLUtil.h"

namespace Ice::SSL::Apple
{
    std::string sslErrorToString(CFErrorRef);
    std::string sslErrorToString(OSStatus);

    //
    // Read certificate from a file.
    //
    ICE_API CFArrayRef loadCertificateChain(
        const std::string&,
        const std::string&,
        const std::string&,
        const std::string&,
        const std::string&);

    ICE_API CFArrayRef loadCACertificates(const std::string&);
    ICE_API CFArrayRef findCertificateChain(const std::string&, const std::string&, const std::string&);

}
#endif

#endif
