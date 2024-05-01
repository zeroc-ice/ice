//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_SECURE_TRANSPORT_ENGINE_H
#define ICESSL_SECURE_TRANSPORT_ENGINE_H

#ifdef __APPLE__

#    include "Ice/ClientAuthenticationOptions.h"
#    include "Ice/InstanceF.h"
#    include "Ice/ServerAuthenticationOptions.h"
#    include "Ice/UniqueRef.h"
#    include "SSLEngine.h"

#    include <Security/SecureTransport.h>
#    include <Security/Security.h>

namespace IceSSL::SecureTransport
{
    class SSLEngine final : public IceSSL::SSLEngine, public std::enable_shared_from_this<SSLEngine>
    {
    public:
        SSLEngine(const IceInternal::InstancePtr&);

        void initialize() final;
        void destroy() final;

        Ice::SSL::ClientAuthenticationOptions createClientAuthenticationOptions(const std::string& host) const final;
        Ice::SSL::ServerAuthenticationOptions createServerAuthenticationOptions() const final;
        SSLContextRef newContext(bool) const;
        bool validationCallback(SecTrustRef trust, const IceSSL::ConnectionInfoPtr&, const std::string&) const;

        std::string getCipherName(SSLCipherSuite) const;

    private:
        IceInternal::UniqueRef<CFArrayRef> _certificateAuthorities;
        IceInternal::UniqueRef<CFArrayRef> _chain;
    };
}
#endif

#endif
