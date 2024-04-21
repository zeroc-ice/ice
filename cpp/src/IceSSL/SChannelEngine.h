//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_SCHANNEL_ENGINE_H
#define ICESSL_SCHANNEL_ENGINE_H

#ifdef _WIN32

#    include "Ice/InstanceF.h"
#    include "Ice/SSL.h"
#    include "Ice/SSLConnectionInfo.h"
#    include "SChannelEngineF.h"
#    include "SSLEngine.h"

#    include <mutex>
#    include <string>
#    include <vector>

namespace IceSSL::SChannel
{
    class SSLEngine final : public IceSSL::SSLEngine, public std::enable_shared_from_this<SSLEngine>
    {
    public:
        SSLEngine(const IceInternal::InstancePtr&);

        //
        // Setup the engine.
        //
        void initialize() final;

        //
        // Destroy the engine.
        //
        void destroy() final;

        std::string getCipherName(ALG_ID) const;

        Ice::SSL::ClientAuthenticationOptions createClientAuthenticationOptions() const;
        Ice::SSL::ServerAuthenticationOptions createServerAuthenticationOptions() const;

    private:
        bool validationCallback(CtxtHandle ssl, const IceSSL::ConnectionInfoPtr&, bool incoming) const;
        std::string errorStatusToString(DWORD errorStatus) const;
        CredHandle newCredentialsHandle(bool) const;

        std::vector<PCCERT_CONTEXT> _allCerts;
        std::vector<PCCERT_CONTEXT> _importedCerts;

        std::vector<HCERTSTORE> _stores;
        HCERTSTORE _rootStore;

        HCERTCHAINENGINE _chainEngine;
        const bool _strongCrypto;
        CredHandle _clientCredentials;
        CredHandle _serverCredentials;
        mutable std::mutex _mutex;
    };
}

#endif

#endif
