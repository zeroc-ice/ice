//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_SECURE_TRANSPORT_ENGINE_H
#define ICESSL_SECURE_TRANSPORT_ENGINE_H

#ifdef __APPLE__

#    include "Ice/InstanceF.h"
#    include "Ice/UniqueRef.h"
#    include "SSLEngine.h"

#    include <Security/SecureTransport.h>
#    include <Security/Security.h>

namespace IceSSL::SecureTransport
{
    class SSLEngine final : public IceSSL::SSLEngine
    {
    public:
        SSLEngine(const IceInternal::InstancePtr&);

        void initialize() final;
        void destroy() final;
        IceInternal::TransceiverPtr
        createTransceiver(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool) final;

        SSLContextRef newContext(bool);
        CFArrayRef getCertificateAuthorities() const;
        std::string getCipherName(SSLCipherSuite) const;

    private:
        IceInternal::UniqueRef<CFArrayRef> _certificateAuthorities;
        IceInternal::UniqueRef<CFArrayRef> _chain;
    };
}
#endif

#endif
