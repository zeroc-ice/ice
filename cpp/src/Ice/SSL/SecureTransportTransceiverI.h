// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_SECURE_TRANSPORT_TRANSCEIVER_I_H
#define ICE_SSL_SECURE_TRANSPORT_TRANSCEIVER_I_H

#ifdef __APPLE__

#    include "../Network.h"
#    include "../Transceiver.h"
#    include "../UniqueRef.h"
#    include "Ice/Config.h"
#    include "Ice/SSL/ClientAuthenticationOptions.h"
#    include "Ice/SSL/ServerAuthenticationOptions.h"
#    include "SSLInstanceF.h"
#    include "SecureTransportEngineF.h"

#    include <CoreFoundation/CoreFoundation.h>
#    include <Security/SecureTransport.h>
#    include <Security/Security.h>

namespace Ice::SSL::SecureTransport
{
    class TransceiverI final : public IceInternal::Transceiver
    {
    public:
        TransceiverI(
            const InstancePtr&,
            IceInternal::TransceiverPtr,
            std::string,
            const Ice::SSL::ServerAuthenticationOptions&);
        TransceiverI(
            const InstancePtr&,
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

        [[nodiscard]] OSStatus writeRaw(const std::byte*, size_t*) const;
        [[nodiscard]] OSStatus readRaw(std::byte*, size_t*) const;

    private:
        const InstancePtr _instance;
        const SSLEnginePtr _engine;
        const std::string _host;
        const std::string _adapterName;
        const bool _incoming;
        const IceInternal::TransceiverPtr _delegate;

        IceInternal::UniqueRef<SSLContextRef> _ssl;
        IceInternal::UniqueRef<SecTrustRef> _trust;
        bool _connected;

        enum SSLWantFlags
        {
            SSLWantRead = 0x1,
            SSLWantWrite = 0x2
        };

        mutable std::uint8_t _tflags;
        IceInternal::UniqueRef<SecCertificateRef> _peerCertificate;
        size_t _buffered;
        std::function<void(SSLContextRef, const std::string&)> _sslNewSessionCallback;
        std::function<bool(SecTrustRef trust, const Ice::SSL::ConnectionInfoPtr& info)>
            _remoteCertificateValidationCallback;
        std::function<CFArrayRef(const std::string&)> _localCertificateSelectionCallback;
        SSLAuthenticate _clientCertificateRequired;
        CFArrayRef _certificates;
        CFArrayRef _trustedRootCertificates;
    };
    using TransceiverIPtr = std::shared_ptr<TransceiverI>;

}
#endif

#endif
