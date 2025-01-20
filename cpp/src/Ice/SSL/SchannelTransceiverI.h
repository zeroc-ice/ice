// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_SCHANNEL_TRANSCEIVER_I_H
#define ICE_SSL_SCHANNEL_TRANSCEIVER_I_H

#ifdef _WIN32

#    include "../Network.h"
#    include "../StreamSocket.h"
#    include "../Transceiver.h"
#    include "../WSTransceiver.h"
#    include "Ice/Buffer.h"
#    include "Ice/Config.h"
#    include "Ice/SSL/Config.h"
#    include "SSLInstanceF.h"
#    include "SchannelEngineF.h"

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
        [[nodiscard]] IceInternal::NativeInfoPtr getNativeInfo() final;
        [[nodiscard]] IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&) final;
        [[nodiscard]] IceInternal::SocketOperation closing(bool, std::exception_ptr) final;
        void close();
        [[nodiscard]] IceInternal::SocketOperation write(IceInternal::Buffer&) final;
        [[nodiscard]] IceInternal::SocketOperation read(IceInternal::Buffer&) final;
        [[nodiscard]] bool startWrite(IceInternal::Buffer&) final;
        void finishWrite(IceInternal::Buffer&) final;
        void startRead(IceInternal::Buffer&) final;
        void finishRead(IceInternal::Buffer&) final;
        [[nodiscard]] std::string protocol() const final;
        [[nodiscard]] std::string toString() const final;
        [[nodiscard]] std::string toDetailedString() const final;
        [[nodiscard]] Ice::ConnectionInfoPtr
        getInfo(bool incoming, std::string adapterName, std::string connectionId) const final;
        void checkSendSize(const IceInternal::Buffer&) final;
        void setBufferSize(int rcvSize, int sndSize) final;

    private:
        [[nodiscard]] IceInternal::SocketOperation sslHandshake(SecBuffer* initialBuffer = nullptr);

        [[nodiscard]] size_t decryptMessage(IceInternal::Buffer&);
        [[nodiscard]] size_t encryptMessage(IceInternal::Buffer&);

        [[nodiscard]] bool writeRaw(IceInternal::Buffer&);
        [[nodiscard]] bool readRaw(IceInternal::Buffer&);

        enum State
        {
            StateNotInitialized,
            StateHandshakeNotStarted,
            StateHandshakeRenegotiateStarted,
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
        bool _sslConnectionRenegotiating;

        // Buffered encrypted data that has not been written.
        IceInternal::Buffer _writeBuffer;
        size_t _bufferedW;

        // Buffered data that has not been decrypted.
        IceInternal::Buffer _readBuffer;

        // Extra buffer used for SSL renegotiation.
        IceInternal::Buffer _extraBuffer;

        // Buffered data that was decrypted but not yet processed.
        IceInternal::Buffer _readUnprocessed;

        std::function<SCH_CREDENTIALS(const std::string&)> _localCredentialsSelectionCallback;
        std::function<void(CtxtHandle context, const std::string& host)> _sslNewSessionCallback;
        SecPkgContext_StreamSizes _sizes;
        std::string _cipher;
        PCCERT_CONTEXT _peerCertificate;
        std::function<bool(CtxtHandle, const Ice::SSL::ConnectionInfoPtr&)> _remoteCertificateValidationCallback;
        bool _clientCertificateRequired;
        SCH_CREDENTIALS _credentials;
        std::vector<PCCERT_CONTEXT> _allCerts;
        CredHandle _credentialsHandle;
        HCERTSTORE _rootStore;
        CtxtHandle _ssl;

        // The chain engine used to verify the peer certificate. If the user has not provided a remote certificate
        // validation callback, we use this chain engine to validate the peer certificate.
        // When the user provides a trusted root certificates stores, this chain engine is configured to exclusively
        // trust the users provided root store.
        HCERTCHAINENGINE _chainEngine;
    };
    using TransceiverIPtr = std::shared_ptr<TransceiverI>;
}
#endif

#endif
