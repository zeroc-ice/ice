// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_ENGINE_H
#define ICE_SSL_ENGINE_H

#include "../Network.h"
#include "../TransceiverF.h"
#include "Ice/CommunicatorF.h"
#include "Ice/Initialize.h"
#include "Ice/InstanceF.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "SSLEngineF.h"
#include "SSLInstanceF.h"
#include "SSLUtil.h"
#include "TrustManagerF.h"

#include <mutex>

namespace Ice::SSL
{
    class SSLEngine
    {
    public:
        SSLEngine(const IceInternal::InstancePtr&);
        ~SSLEngine();

        [[nodiscard]] Ice::LoggerPtr getLogger() const;
        [[nodiscard]] Ice::PropertiesPtr getProperties() const;
        [[nodiscard]] Ice::InitializationData getInitializationData() const;

        [[nodiscard]] IceInternal::InstancePtr instance() const { return _instance; }

        // Setup the engine.
        virtual void initialize() = 0;

        // Destroy the engine.
        virtual void destroy() = 0;

        // Verify peer certificate.
        virtual void verifyPeer(const ConnectionInfoPtr&) const;

        [[nodiscard]] virtual ClientAuthenticationOptions
        createClientAuthenticationOptions(const std::string&) const = 0;
        [[nodiscard]] virtual ServerAuthenticationOptions createServerAuthenticationOptions() const = 0;

        [[nodiscard]] bool getCheckCertName() const;
        [[nodiscard]] int getVerifyPeer() const;
        [[nodiscard]] int securityTraceLevel() const;
        [[nodiscard]] bool getRevocationCheckCacheOnly() const;
        [[nodiscard]] int getRevocationCheck() const;
        [[nodiscard]] std::string securityTraceCategory() const;

    protected:
        mutable std::mutex _mutex;

    private:
        const IceInternal::InstancePtr _instance;
        const TrustManagerPtr _trustManager;

        std::string _password;

        bool _checkCertName;
        int _verifyPeer;
        int _securityTraceLevel;
        std::string _securityTraceCategory;
        const bool _revocationCheckCacheOnly{false};
        const int _revocationCheck{0};
    };
}

#endif
