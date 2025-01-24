// Copyright (c) ZeroC, Inc.

#ifndef ICESSL_OPENSSL_ENGINE_H
#define ICESSL_OPENSSL_ENGINE_H

#include "Ice/BuiltinSequences.h"
#include "Ice/InstanceF.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "SSLEngine.h"
#include "SSLInstanceF.h"
#include "SSLUtil.h"

namespace Ice::SSL::OpenSSL
{
    class SSLEngine final : public Ice::SSL::SSLEngine
    {
    public:
        SSLEngine(const IceInternal::InstancePtr&);
        ~SSLEngine();

        void initialize() final;
        void destroy() final;
        [[nodiscard]] std::string sslErrors() const;
        [[nodiscard]] std::string password() const { return _password; }
        [[nodiscard]] Ice::SSL::ClientAuthenticationOptions
        createClientAuthenticationOptions(const std::string&) const final;
        [[nodiscard]] Ice::SSL::ServerAuthenticationOptions createServerAuthenticationOptions() const final;

    private:
        bool validationCallback(bool, X509_STORE_CTX*, const Ice::SSL::ConnectionInfoPtr&) const;
        // The SSL_CTX object configured with IceSSL properties. This object is shared across all SSL incoming and
        // outgoing connections that do not specify custom ServerAuthenticationOptions or ClientAuthenticationOptions.
        SSL_CTX* _ctx{nullptr};
        std::string _password;
    };

}

#endif
