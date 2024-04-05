//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_OPENSSL_ENGINE_H
#define ICESSL_OPENSSL_ENGINE_H

#include "InstanceF.h"
#include "OpenSSLUtil.h"
#include "SSLEngine.h"

#include "Ice/BuiltinSequences.h"

namespace IceSSL
{
    namespace OpenSSL
    {
        class SSLEngine final : public IceSSL::SSLEngine
        {
        public:
            SSLEngine(const Ice::CommunicatorPtr&);
            ~SSLEngine();

            void initialize() final;
            void destroy() final;
            IceInternal::TransceiverPtr
            createTransceiver(const IceSSL::InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool)
                final;

#ifndef OPENSSL_NO_DH
            DH* dhParams(int);
#endif

            SSL_CTX* context() const;
            void context(SSL_CTX*);
            std::string sslErrors() const;

        private:
            void cleanup();
            SSL_METHOD* getMethod();

            SSL_CTX* _ctx;

#ifndef OPENSSL_NO_DH
            IceSSL::OpenSSL::DHParamsPtr _dhParams;
#endif
        };

    } // OpenSSL namespace end

} // IceSSL namespace endif

#endif
