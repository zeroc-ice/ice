//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_OPENSSL_ENGINE_H
#define ICESSL_OPENSSL_ENGINE_H

#include "../Ice/InstanceF.h"
#include "Ice/BuiltinSequences.h"
#include "Ice/ClientAuthenticationOptions.h"
#include "Ice/ServerAuthenticationOptions.h"
#include "OpenSSLUtil.h"
#include "SSLEngine.h"
#include "SSLInstanceF.h"

namespace IceSSL::OpenSSL
{
    class SSLEngine final : public IceSSL::SSLEngine
    {
    public:
        SSLEngine(const IceInternal::InstancePtr&);
        ~SSLEngine();

        void initialize() final;
        void destroy() final;
        std::string sslErrors() const;
        std::string password() const { return _password; }
        Ice::SSL::ClientAuthenticationOptions createClientAuthenticationOptions(const std::string&) const final;
        Ice::SSL::ServerAuthenticationOptions createServerAuthenticationOptions() const final;

    private:
        bool validationCallback(bool, X509_STORE_CTX*, const IceSSL::ConnectionInfoPtr&) const;
        // The SSL_CTX object configured with IceSSL properties. This object is shared across all SSL incoming and
        // outgoing connections that do not specify custom ServerAuthenticationOptions or ClientAuthenticationOptions.
        SSL_CTX* _ctx;
        std::string _password;
    };

}

#endif
