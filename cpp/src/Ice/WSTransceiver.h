// Copyright (c) ZeroC, Inc.

#ifndef ICE_WS_TRANSCEIVER_I_H
#define ICE_WS_TRANSCEIVER_I_H

#include "HttpParser.h"
#include "Ice/Buffer.h"
#include "Ice/Logger.h"
#include "Network.h"
#include "ProtocolInstance.h"
#include "Transceiver.h"

namespace IceInternal
{
    class ConnectorI;
    class AcceptorI;

    class WSTransceiver final : public Transceiver
    {
    public:
        WSTransceiver(ProtocolInstancePtr, TransceiverPtr, std::string, std::string);
        WSTransceiver(ProtocolInstancePtr, TransceiverPtr);
        ~WSTransceiver();

        NativeInfoPtr getNativeInfo() final;
#if defined(ICE_USE_IOCP)
        AsyncInfo* getAsyncInfo(SocketOperation);
#endif

        SocketOperation initialize(Buffer&, Buffer&) final;
        SocketOperation closing(bool, std::exception_ptr) final;
        void close() final;
        SocketOperation write(Buffer&) final;
        SocketOperation read(Buffer&) final;
#if defined(ICE_USE_IOCP)
        bool startWrite(Buffer&) final;
        void finishWrite(Buffer&) final;
        void startRead(Buffer&) final;
        void finishRead(Buffer&) final;
#endif
        [[nodiscard]] std::string protocol() const final;
        [[nodiscard]] std::string toString() const final;
        [[nodiscard]] std::string toDetailedString() const final;
        [[nodiscard]] Ice::ConnectionInfoPtr
        getInfo(bool incoming, std::string adapterName, std::string connectionId) const final;
        void checkSendSize(const Buffer&) final;
        void setBufferSize(int rcvSize, int sndSize) final;

    private:
        void handleRequest(Buffer&);
        void handleResponse();

        bool preRead(Buffer&);
        bool postRead(Buffer&);

        bool preWrite(Buffer&);
        bool postWrite(Buffer&);

        bool readBuffered(Buffer::Container::size_type);
        void prepareWriteHeader(std::uint8_t, Buffer::Container::size_type);

        friend class WSConnector;
        friend class WSAcceptor;

        const ProtocolInstancePtr _instance;
        const TransceiverPtr _delegate;
        const std::string _host;
        const std::string _resource;
        const bool _incoming;

        enum State
        {
            StateInitializeDelegate,
            StateConnected,
            StateUpgradeRequestPending,
            StateUpgradeResponsePending,
            StateOpened,
            StatePingPending,
            StatePongPending,
            StateClosingRequestPending,
            StateClosingResponsePending,
            StateClosed
        };

        State _state;
        State _nextState;

        HttpParserPtr _parser;
        std::string _key;

        enum ReadState
        {
            ReadStateOpcode,
            ReadStateHeader,
            ReadStateControlFrame,
            ReadStatePayload,
        };

        ReadState _readState;
        Buffer _readBuffer;
        Buffer::Container::iterator _readI;
        const Buffer::Container::size_type _readBufferSize;

        bool _readLastFrame;
        int _readOpCode;
        size_t _readHeaderLength;
        size_t _readPayloadLength;
        Buffer::Container::iterator _readStart;
        Buffer::Container::iterator _readFrameStart;
        std::byte _readMask[4];

        enum WriteState
        {
            WriteStateHeader,
            WriteStatePayload,
            WriteStateControlFrame,
        };

        WriteState _writeState;
        Buffer _writeBuffer;
        const Buffer::Container::size_type _writeBufferSize;
        std::byte _writeMask[4];
        size_t _writePayloadLength;

        bool _readPending;
        bool _writePending;

        bool _closingInitiator;
        int _closingReason;

        std::vector<std::uint8_t> _pingPayload;
    };
}

#endif
