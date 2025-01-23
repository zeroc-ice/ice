// Copyright (c) ZeroC, Inc.

#ifndef ICESSL_OPENSSL_TRANSCEIVER_I_H
#define ICESSL_OPENSSL_TRANSCEIVER_I_H

#include "../Network.h"
#include "../StreamSocket.h"
#include "../Transceiver.h"
#include "Ice/Config.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "OpenSSLEngineF.h"
#include "SSLInstanceF.h"
#include "SSLUtil.h"

#include <openssl/ssl.h>

using SSL = struct ssl_st;
using BIO = struct bio_st;

namespace Ice::SSL::OpenSSL
{
    class TransceiverI final : public IceInternal::Transceiver
    {
    public:
        TransceiverI(
            InstancePtr,
            IceInternal::TransceiverPtr,
            std::string,
            const Ice::SSL::ServerAuthenticationOptions&);
        TransceiverI(
            InstancePtr,
            IceInternal::TransceiverPtr,
            std::string,
            const Ice::SSL::ClientAuthenticationOptions&);

        ~TransceiverI();
        [[nodiscard]] IceInternal::NativeInfoPtr getNativeInfo() final;

        [[nodiscard]] IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&) final;
        [[nodiscard]] IceInternal::SocketOperation closing(bool, std::exception_ptr) final;
        void close() final;
        [[nodiscard]] IceInternal::SocketOperation write(IceInternal::Buffer&) final;
        [[nodiscard]] IceInternal::SocketOperation read(IceInternal::Buffer&) final;
        [[nodiscard]] std::string protocol() const final;
        [[nodiscard]] std::string toString() const final;
        [[nodiscard]] std::string toDetailedString() const final;
        [[nodiscard]] Ice::ConnectionInfoPtr
        getInfo(bool incoming, std::string adapterName, std::string connectionId) const final;
        void checkSendSize(const IceInternal::Buffer&) final;
        void setBufferSize(int rcvSize, int sndSize) final;

        int verifyCallback(int, X509_STORE_CTX*);

    private:
        bool receive();
        bool send();

        friend class Ice::SSL::OpenSSL::SSLEngine;

        const InstancePtr _instance;
        const Ice::SSL::OpenSSL::SSLEnginePtr _engine;
        const std::string _host;
        const std::string _adapterName;
        const bool _incoming;
        const IceInternal::TransceiverPtr _delegate;
        bool _connected;
        std::string _cipher;
        X509* _peerCertificate;
        ::SSL* _ssl;
        SSL_CTX* _sslCtx;
        BIO* _memBio;
        IceInternal::Buffer _writeBuffer;
        IceInternal::Buffer _readBuffer;
        int _sentBytes;
        size_t _maxSendPacketSize;
        size_t _maxRecvPacketSize;
        std::function<SSL_CTX*(const std::string&)> _localSSLContextSelectionCallback;
        std::function<bool(bool, X509_STORE_CTX*, const Ice::SSL::ConnectionInfoPtr&)>
            _remoteCertificateVerificationCallback;
        std::function<void(::SSL*, const std::string&)> _sslNewSessionCallback;
        std::exception_ptr _verificationException;
    };
    using TransceiverIPtr = std::shared_ptr<TransceiverI>;

}
#endif
