// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_TRANSCEIVER_H
#define ICE_STREAM_TRANSCEIVER_H

#include <Ice/ProtocolInstanceF.h>
#include <Ice/Transceiver.h>
#include <Ice/Network.h>
#include <Ice/WSTransceiver.h>

namespace IceInternal
{

class StreamConnector;
class StreamAcceptor;

class StreamTransceiver : public Transceiver, public NativeInfo, public WSTransceiverDelegate
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
    virtual SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual SocketOperation write(Buffer&);
    virtual SocketOperation read(Buffer&, bool&);

    virtual bool startWrite(Buffer&);
    virtual void finishWrite(Buffer&);
    virtual void startRead(Buffer&);
    virtual void finishRead(Buffer&, bool&);

    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual Ice::ConnectionInfoPtr getWSInfo(const Ice::HeaderDict&) const;
    virtual void checkSendSize(const Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

private:

    StreamTransceiver(const ProtocolInstancePtr&, SOCKET, bool);
    virtual ~StreamTransceiver();

    void connect(const Address&);
    bool checkIfErrorOrCompleted(SocketOperation, Windows::Foundation::IAsyncInfo^, int = 0);
    void fillConnectionInfo(const Ice::IPConnectionInfoPtr&) const;

    friend class StreamConnector;
    friend class StreamAcceptor;

    const ProtocolInstancePtr _instance;

    State _state;
    std::string _desc;
    bool _verified;
    Address _connectAddr;

    AsyncInfo _read;
    AsyncInfo _write;
    int _maxSendPacketSize;
    int _maxReceivePacketSize;

    Windows::Storage::Streams::DataReader^ _reader;
    Windows::Storage::Streams::DataWriter^ _writer;

    Windows::Foundation::AsyncOperationCompletedHandler<unsigned int>^ _readOperationCompletedHandler;
    Windows::Foundation::AsyncOperationCompletedHandler<unsigned int>^ _writeOperationCompletedHandler;
};

}

#endif
