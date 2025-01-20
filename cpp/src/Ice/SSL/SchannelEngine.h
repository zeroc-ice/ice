// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_SCHANNEL_ENGINE_H
#define ICE_SSL_SCHANNEL_ENGINE_H

#ifdef _WIN32

#    include "Ice/InstanceF.h"
#    include "Ice/SSL/ClientAuthenticationOptions.h"
#    include "Ice/SSL/ConnectionInfo.h"
#    include "Ice/SSL/ServerAuthenticationOptions.h"
#    include "SSLEngine.h"
#    include "SchannelEngineF.h"

#    include <mutex>
#    include <string>
#    include <vector>

namespace Ice::SSL::Schannel
{
    class SSLEngine final : public Ice::SSL::SSLEngine, public std::enable_shared_from_this<SSLEngine>
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

        [[nodiscard]] std::string getCipherName(ALG_ID) const;

        [[nodiscard]] Ice::SSL::ClientAuthenticationOptions
        createClientAuthenticationOptions(const std::string&) const final;
        [[nodiscard]] Ice::SSL::ServerAuthenticationOptions createServerAuthenticationOptions() const final;
        [[nodiscard]] static bool
        validationCallback(HCERTCHAINENGINE chainEngine, CtxtHandle, bool, const std::string&, bool, int, bool);

    private:
        std::vector<PCCERT_CONTEXT> _allCerts;
        std::vector<PCCERT_CONTEXT> _importedCerts;

        std::vector<HCERTSTORE> _stores;
        HCERTSTORE _rootStore;

        HCERTCHAINENGINE _chainEngine;
        mutable std::mutex _mutex;
    };
}

#endif

#endif
