//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_ENGINE_H
#define ICESSL_ENGINE_H

#include "../Ice/Network.h"
#include "../Ice/TransceiverF.h"
#include "Ice/Certificate.h"
#include "Ice/CommunicatorF.h"
#include "Ice/InstanceF.h"
#include "SSLEngineF.h"
#include "SSLInstanceF.h"
#include "SSLUtil.h"
#include "TrustManagerF.h"

#include <mutex>

namespace IceSSL
{
    class ICE_API SSLEngine
    {
    public:
        SSLEngine(const IceInternal::InstancePtr&);

        Ice::LoggerPtr getLogger() const;
        Ice::PropertiesPtr getProperties() const;

        IceInternal::InstancePtr instance() const { return _instance; }

        // Setup the engine.
        virtual void initialize() = 0;

        // Destroy the engine.
        virtual void destroy() = 0;

        // Create a transceiver using the engine specific implementation.
        virtual IceInternal::TransceiverPtr
        createTransceiver(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool) = 0;

        // Verify peer certificate.
        virtual void verifyPeer(const std::string&, const ConnectionInfoPtr&, const std::string&);
        void verifyPeerCertName(const std::string&, const ConnectionInfoPtr&);

        bool getCheckCertName() const;
        bool getServerNameIndication() const;
        int getVerifyPeer() const;
        int securityTraceLevel() const;
        bool getRevocationCheckCacheOnly() const;
        int getRevocationCheck() const;
        std::string securityTraceCategory() const;

    protected:
        mutable std::mutex _mutex;

    private:
        const IceInternal::InstancePtr _instance;
        const TrustManagerPtr _trustManager;

        std::string _password;

        bool _checkCertName;
        bool _serverNameIndication;
        int _verifyPeer;
        int _securityTraceLevel;
        std::string _securityTraceCategory;
        const bool _revocationCheckCacheOnly;
        const int _revocationCheck;
    };
}

#endif
