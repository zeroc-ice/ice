//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_PLUGIN_I_H
#define ICESSL_PLUGIN_I_H

#include "Ice/CommunicatorF.h"
#include "IceSSL/ConnectionInfo.h"
#include "IceSSL/Plugin.h"
#include "SSLEngineF.h"

namespace IceSSL
{
    class ExtendedConnectionInfo : public ConnectionInfo
    {
    public:
        TrustError errorCode;
        std::string host;
    };
    using ExtendedConnectionInfoPtr = std::shared_ptr<ExtendedConnectionInfo>;

    // TODO: This class provides new certificate virtual methods that cannot be added directly to the certificate class
    // without breaking binary compatibility. The class can be removed once the relevant methods can be marked as
    // virtual in the certificate class in the next major release (3.8.x).
    class ICE_API CertificateExtendedInfo
    {
    public:
        virtual unsigned int getKeyUsage() const = 0;
        virtual unsigned int getExtendedKeyUsage() const = 0;
    };
}

#endif
