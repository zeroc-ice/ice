// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_WS_TRANSCEIVER_I_H
#define ICE_WS_TRANSCEIVER_I_H

#include <IceWS/InstanceF.h>
#include <IceWS/Plugin.h>
#include <IceWS/Util.h>

#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
#include <Ice/Transceiver.h>
#include <Ice/Network.h>
#include <Ice/Buffer.h>

namespace IceWS
{

class ConnectorI;
class AcceptorI;

class TransceiverI : public IceInternal::Transceiver
{
public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();
#ifdef ICE_USE_IOCP
    virtual IceInternal::AsyncInfo* getAsyncInfo(IceInternal::SocketOperation);
#endif

    virtual IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&, bool&);
    virtual IceInternal::SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual IceInternal::SocketOperation write(IceInternal::Buffer&);
    virtual IceInternal::SocketOperation read(IceInternal::Buffer&, bool&);
#ifdef ICE_USE_IOCP
    virtual bool startWrite(IceInternal::Buffer&);
    virtual void finishWrite(IceInternal::Buffer&);
    virtual void startRead(IceInternal::Buffer&);
    virtual void finishRead(IceInternal::Buffer&);
#endif
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const IceInternal::Buffer&, size_t);

private:

    TransceiverI(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, int, const std::string&);
    TransceiverI(const InstancePtr&, const IceInternal::TransceiverPtr&);
    virtual ~TransceiverI();

    void handleRequest(IceInternal::Buffer&);
    void handleResponse();

    bool preRead(IceInternal::Buffer&);
    bool postRead(IceInternal::Buffer&);

    bool preWrite(IceInternal::Buffer&);
    bool postWrite(IceInternal::Buffer&);

    bool readBuffered(IceInternal::Buffer::Container::size_type);
    void prepareWriteHeader(Ice::Byte, IceInternal::Buffer::Container::size_type);

    friend class ConnectorI;
    friend class AcceptorI;

    const InstancePtr _instance;
    const IceInternal::TransceiverPtr _delegate;
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
    IceInternal::Buffer _readBuffer;
    IceInternal::Buffer::Container::iterator _readI;
    const IceInternal::Buffer::Container::size_type _readBufferSize;

    bool _readLastFrame;
    int _readOpCode;
    size_t _readHeaderLength;
    size_t _readPayloadLength;
    IceInternal::Buffer::Container::iterator _readStart;
    unsigned char _readMask[4];

    enum WriteState
    {
        WriteStateHeader,
        WriteStatePayload,
        WriteStateControlFrame,
    };

    WriteState _writeState;
    IceInternal::Buffer _writeBuffer;
    const IceInternal::Buffer::Container::size_type _writeBufferSize;
    unsigned char _writeMask[4];
    size_t _writePayloadLength;

    bool _closingInitiator;
    int _closingReason;

    bool _readPending;
    bool _writePending;

    std::vector<Ice::Byte> _pingPayload;
};
typedef IceUtil::Handle<TransceiverI> TransceiverIPtr;

}

#endif
