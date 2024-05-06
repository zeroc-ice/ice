//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_SCHANNEL_TRANSCEIVER_I_H
#define ICE_SSL_SCHANNEL_TRANSCEIVER_I_H

#ifdef _WIN32

#    include "../Ice/Network.h"
#    include "../Ice/StreamSocket.h"
#    include "../Ice/Transceiver.h"
#    include "../Ice/WSTransceiver.h"
#    include "Ice/Buffer.h"
#    include "Ice/Config.h"
#    include "SSLInstanceF.h"
#    include "SchannelEngineF.h"

#    ifdef SECURITY_WIN32
#        undef SECURITY_WIN32
#    endif

#    ifdef SECURITY_KERNEL
#        undef SECURITY_KERNEL
#    endif

#    define SECURITY_WIN32 1
#    include <schannel.h>
#    include <security.h>
#    include <sspi.h>
#    undef SECURITY_WIN32

namespace Ice::SSL::Schannel
{
    class TransceiverI final : public IceInternal::Transceiver
    {
    public:
        TransceiverI(
            const InstancePtr&,
            const IceInternal::TransceiverPtr&,
            const std::string&,
            const Ice::SSL::ServerAuthenticationOptions&);
        TransceiverI(
            const InstancePtr&,
            const IceInternal::TransceiverPtr&,
            const std::string&,
            const Ice::SSL::ClientAuthenticationOptions&);
        ~TransceiverI();
        IceInternal::NativeInfoPtr getNativeInfo() final;

        IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&) final;
        IceInternal::SocketOperation closing(bool, std::exception_ptr) final;
        void close();
        IceInternal::SocketOperation write(IceInternal::Buffer&) final;
        IceInternal::SocketOperation read(IceInternal::Buffer&) final;
#    ifdef ICE_USE_IOCP
        bool startWrite(IceInternal::Buffer&) final;
        void finishWrite(IceInternal::Buffer&) final;
        void startRead(IceInternal::Buffer&) final;
        void finishRead(IceInternal::Buffer&) final;
#    endif
        bool isWaitingToBeRead() const noexcept final;
        std::string protocol() const final;
        std::string toString() const final;
        std::string toDetailedString() const final;
        Ice::ConnectionInfoPtr getInfo() const final;
        void checkSendSize(const IceInternal::Buffer&) final;
        void setBufferSize(int rcvSize, int sndSize) final;

    private:
        IceInternal::SocketOperation sslHandshake();

        size_t decryptMessage(IceInternal::Buffer&);
        size_t encryptMessage(IceInternal::Buffer&);

        bool writeRaw(IceInternal::Buffer&);
        bool readRaw(IceInternal::Buffer&);

        enum State
        {
            StateNotInitialized,
            StateHandshakeNotStarted,
            StateHandshakeReadContinue,
            StateHandshakeWriteContinue,
            StateHandshakeWriteNoContinue,
            StateHandshakeComplete
        };

        const InstancePtr _instance;
        const Ice::SSL::Schannel::SSLEnginePtr _engine;
        const std::string _host;
        const std::string _adapterName;
        const bool _incoming;
        const IceInternal::TransceiverPtr _delegate;
        State _state;
        DWORD _ctxFlags;

        //
        // Buffered encrypted data that has not been written.
        //
        IceInternal::Buffer _writeBuffer;
        size_t _bufferedW;

        //
        // Buffered data that has not been decrypted.
        //
        IceInternal::Buffer _readBuffer;

        //
        // Buffered data that was decrypted but not yet processed.
        //
        IceInternal::Buffer _readUnprocessed;

        std::function<PCCERT_CONTEXT(const std::string&)> _localCertificateSelectionCallback;
        std::function<void(CtxtHandle context, const std::string& host)> _sslNewSessionCallback;
        SecPkgContext_StreamSizes _sizes;
        std::string _cipher;
        std::vector<Ice::SSL::CertificatePtr> _peerCertificates;
        std::function<bool(CtxtHandle, const Ice::SSL::ConnectionInfoPtr&)> _remoteCertificateValidationCallback;
        bool _clientCertificateRequired;
        PCCERT_CONTEXT _certificate;
        HCERTSTORE _rootStore;
        CredHandle _credentials;
        CtxtHandle _ssl;
    };
    using TransceiverIPtr = std::shared_ptr<TransceiverI>;
}
#endif

#endif
