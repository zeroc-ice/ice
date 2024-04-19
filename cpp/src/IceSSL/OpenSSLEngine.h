//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_OPENSSL_ENGINE_H
#define ICESSL_OPENSSL_ENGINE_H

#include "../Ice/InstanceF.h"
#include "Ice/BuiltinSequences.h"
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
        SSL_CTX* context() const;
        std::string sslErrors() const;
        std::string password() const { return _password; }

    private:
        SSL_CTX* _ctx;
        std::string _password;
    };

}

#endif
