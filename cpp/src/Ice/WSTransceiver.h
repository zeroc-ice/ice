// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_WS_TRANSCEIVER_I_H
#define ICE_WS_TRANSCEIVER_I_H

#include <Ice/ProtocolInstance.h>
#include <Ice/HttpParser.h>

#include <Ice/LoggerF.h>
#include <Ice/Transceiver.h>
#include <Ice/Network.h>
#include <Ice/Buffer.h>

namespace IceInternal
{

class ConnectorI;
class AcceptorI;

//
// Delegate interface implemented by TcpTransceiver or IceSSL::Transceiver or any transport that WS can
// delegate to.
//
class ICE_API WSTransceiverDelegate : virtual public IceUtil::Shared
{
public:

    virtual Ice::ConnectionInfoPtr getWSInfo(const Ice::HeaderDict&) const = 0;
};

class WSTransceiver : public Transceiver
{
public:

    virtual NativeInfoPtr getNativeInfo();
#if defined(ICE_USE_IOCP)
    virtual AsyncInfo* getAsyncInfo(SocketOperation);
#elif defined(ICE_OS_WINRT)
    virtual void setCompletedHandler(SocketOperationCompletedHandler^);
#endif

    virtual SocketOperation initialize(Buffer&, Buffer&, bool&);
    virtual SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual SocketOperation write(Buffer&);
    virtual SocketOperation read(Buffer&, bool&);
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    virtual bool startWrite(Buffer&);
    virtual void finishWrite(Buffer&);
    virtual void startRead(Buffer&);
    virtual void finishRead(Buffer&, bool&);
#endif
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

private:

    WSTransceiver(const ProtocolInstancePtr&, const TransceiverPtr&, const std::string&, int, const std::string&);
    WSTransceiver(const ProtocolInstancePtr&, const TransceiverPtr&);
    virtual ~WSTransceiver();

    void handleRequest(Buffer&);
    void handleResponse();

    bool preRead(Buffer&);
    bool postRead(Buffer&);

    bool preWrite(Buffer&);
    bool postWrite(Buffer&);

    bool readBuffered(Buffer::Container::size_type);
    void prepareWriteHeader(Ice::Byte, Buffer::Container::size_type);

    friend class WSConnector;
    friend class WSAcceptor;

    const ProtocolInstancePtr _instance;
    const TransceiverPtr _delegate;
    const std::string _host;
    const int _port;
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
    unsigned char _readMask[4];

    enum WriteState
    {
        WriteStateHeader,
        WriteStatePayload,
        WriteStateControlFrame,
    };

    WriteState _writeState;
    Buffer _writeBuffer;
    const Buffer::Container::size_type _writeBufferSize;
    unsigned char _writeMask[4];
    size_t _writePayloadLength;

    bool _readPending;
    bool _writePending;

    bool _closingInitiator;
    int _closingReason;

    std::vector<Ice::Byte> _pingPayload;
};
typedef IceUtil::Handle<WSTransceiver> WSTransceiverPtr;

}

#endif
