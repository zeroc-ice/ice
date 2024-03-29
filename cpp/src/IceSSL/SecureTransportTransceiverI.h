//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_SECURE_TRANSPORT_TRANSCEIVER_I_H
#define ICESSL_SECURE_TRANSPORT_TRANSCEIVER_I_H

#ifdef __APPLE__

#    include <IceSSL/Config.h>
#    include <IceSSL/InstanceF.h>
#    include <IceSSL/SecureTransportEngineF.h>
#    include <IceSSL/Plugin.h>

#    include "Ice/Transceiver.h"
#    include "Ice/UniqueRef.h"
#    include "Ice/Network.h"

#    include <Security/Security.h>
#    include <Security/SecureTransport.h>
#    include <CoreFoundation/CoreFoundation.h>

namespace IceSSL
{
    namespace SecureTransport
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

    } // SecureTransport namespace end

} // IceSSL namespace end

#endif

#endif
