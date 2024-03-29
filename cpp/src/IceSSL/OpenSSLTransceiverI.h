//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_OPENSSL_TRANSCEIVER_I_H
#define ICESSL_OPENSSL_TRANSCEIVER_I_H

#include "IceSSL/Config.h"
#include "Util.h"
#include "InstanceF.h"
#include "IceSSL/Plugin.h"
#include "OpenSSLEngineF.h"
#include "../Ice/Transceiver.h"
#include "../Ice/Network.h"
#include "../Ice/StreamSocket.h"
#include "../Ice/WSTransceiver.h"

#include <openssl/ssl.h>

typedef struct ssl_st SSL;
typedef struct bio_st BIO;

namespace IceSSL
{
    namespace OpenSSL
    {
        class TransceiverI final : public IceInternal::Transceiver
        {
        public:
            TransceiverI(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool);
            ~TransceiverI();
            IceInternal::NativeInfoPtr getNativeInfo() final;

            IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&) final;
            IceInternal::SocketOperation closing(bool, std::exception_ptr) final;
            void close() final;
            IceInternal::SocketOperation write(IceInternal::Buffer&) final;
            IceInternal::SocketOperation read(IceInternal::Buffer&) final;
#ifdef ICE_USE_IOCP
            bool startWrite(IceInternal::Buffer&) final;
            void finishWrite(IceInternal::Buffer&) final;
            void startRead(IceInternal::Buffer&) final;
            void finishRead(IceInternal::Buffer&) final;
#endif
            std::string protocol() const final;
            std::string toString() const final;
            std::string toDetailedString() const final;
            Ice::ConnectionInfoPtr getInfo() const final;
            void checkSendSize(const IceInternal::Buffer&) final;
            void setBufferSize(int rcvSize, int sndSize) final;

            int verifyCallback(int, X509_STORE_CTX*);

        private:
            bool receive();
            bool send();

            friend class IceSSL::OpenSSL::SSLEngine;

            const InstancePtr _instance;
            const IceSSL::OpenSSL::SSLEnginePtr _engine;
            const std::string _host;
            const std::string _adapterName;
            const bool _incoming;
            const IceInternal::TransceiverPtr _delegate;
            bool _connected;
            std::string _cipher;
            std::vector<IceSSL::CertificatePtr> _certs;
            bool _verified;
            TrustError _trustError;

            SSL* _ssl;
            BIO* _memBio;
            IceInternal::Buffer _writeBuffer;
            IceInternal::Buffer _readBuffer;
            int _sentBytes;
            size_t _maxSendPacketSize;
            size_t _maxRecvPacketSize;
        };
        using TransceiverIPtr = std::shared_ptr<TransceiverI>;

    } // OpenSSL namespace end

} // IceSSL namespace end

#endif
