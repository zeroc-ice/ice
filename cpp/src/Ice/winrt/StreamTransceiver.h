// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_TRANSCEIVER_H
#define ICE_STREAM_TRANSCEIVER_H

#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
#include <Ice/Transceiver.h>
#include <Ice/Network.h>

namespace IceInternal
{

class StreamConnector;
class StreamAcceptor;

std::string typeToString(Ice::Short);

class StreamTransceiver : public Transceiver, public NativeInfo
{
    enum State
    {
        StateNeedConnect,
        StateConnectPending,
        StateConnected
    };

public:

    virtual NativeInfoPtr getNativeInfo();
    virtual void setCompletedHandler(SocketOperationCompletedHandler^);
    
    virtual SocketOperation initialize(Buffer&, Buffer&, bool&);
    virtual SocketOperation initialize();
    virtual SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual SocketOperation write(Buffer&);
    virtual SocketOperation read(Buffer&);

    virtual bool startWrite(Buffer&);
    virtual void finishWrite(Buffer&);
    virtual void startRead(Buffer&);
    virtual void finishRead(Buffer&);

    virtual std::string type() const;
    virtual std::string toString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const Buffer&, size_t);

private:

    StreamTransceiver(const InstancePtr&, Ice::Short, SOCKET, bool);
    virtual ~StreamTransceiver();

    void connect(const Address&);

    bool checkIfErrorOrCompleted(SocketOperation, Windows::Foundation::IAsyncInfo^);

    friend class StreamConnector;
    friend class StreamAcceptor;

    const TraceLevelsPtr _traceLevels;
    const Ice::Short _type;
    const Ice::LoggerPtr _logger;
    const Ice::StatsPtr _stats;
    
    State _state;
    std::string _desc;
    Address _connectAddr;

    AsyncInfo _read;
    AsyncInfo _write;
    int _maxSendPacketSize;
    int _maxReceivePacketSize;

    Windows::Storage::Streams::DataReader^ _reader;
    Windows::Storage::Streams::DataWriter^ _writer;

    SocketOperationCompletedHandler^ _completedHandler;
    Windows::Foundation::AsyncOperationCompletedHandler<unsigned int>^ _readOperationCompletedHandler;
    Windows::Foundation::AsyncOperationCompletedHandler<unsigned int>^ _writeOperationCompletedHandler;
};

}

#endif
