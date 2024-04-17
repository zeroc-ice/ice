//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_SECURE_TRANSPORT_TRANSCEIVER_I_H
#define ICESSL_SECURE_TRANSPORT_TRANSCEIVER_I_H

#ifdef __APPLE__

#    include "../Ice/Network.h"
#    include "../Ice/Transceiver.h"
#    include "Ice/Certificate.h"
#    include "Ice/Config.h"
#    include "Ice/UniqueRef.h"
#    include "SSLInstanceF.h"
#    include "SecureTransportEngineF.h"

#    include <CoreFoundation/CoreFoundation.h>
#    include <Security/SecureTransport.h>
#    include <Security/Security.h>

namespace IceSSL::SecureTransport
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

        bool isWaitingToBeRead() const noexcept final;
        std::string protocol() const final;
        std::string toString() const final;
        std::string toDetailedString() const final;
        Ice::ConnectionInfoPtr getInfo() const final;
        void checkSendSize(const IceInternal::Buffer&) final;
        void setBufferSize(int rcvSize, int sndSize) final;

        OSStatus writeRaw(const std::byte*, size_t*) const;
        OSStatus readRaw(std::byte*, size_t*) const;

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
        size_t _maxSendPacketSize;
        size_t _maxRecvPacketSize;
        std::string _cipher;
        std::vector<CertificatePtr> _certs;
        TrustError _trustError;
        bool _verified;
        size_t _buffered;
    };
    using TransceiverIPtr = std::shared_ptr<TransceiverI>;

}
#endif

#endif
