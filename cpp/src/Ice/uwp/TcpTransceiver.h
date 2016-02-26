// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UWP_TCP_TRANSCEIVER_H
#define ICE_UWP_TCP_TRANSCEIVER_H

#include <Ice/ProtocolInstanceF.h>
#include <Ice/Transceiver.h>
#include <Ice/Network.h>
#include <Ice/WSTransceiver.h>

namespace IceInternal
{

class StreamConnector;
class StreamAcceptor;

class TcpTransceiver : public Transceiver, public NativeInfo, public WSTransceiverDelegate
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

    virtual SocketOperation initialize(Buffer&, Buffer&);
#ifdef ICE_CPP11_MAPPING
	virtual SocketOperation closing(bool, std::exception_ptr);
#else
    virtual SocketOperation closing(bool, const Ice::LocalException&);
#endif
    virtual void close();
    virtual SocketOperation write(Buffer&);
    virtual SocketOperation read(Buffer&);

    virtual bool startWrite(Buffer&);
    virtual void finishWrite(Buffer&);
    virtual void startRead(Buffer&);
    virtual void finishRead(Buffer&);

    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual Ice::ConnectionInfoPtr getWSInfo(const Ice::HeaderDict&) const;
    virtual void checkSendSize(const Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

private:

    TcpTransceiver(const ProtocolInstancePtr&, SOCKET, bool);
    virtual ~TcpTransceiver();

    void connect(const Address&);
    bool checkIfErrorOrCompleted(SocketOperation, Windows::Foundation::IAsyncInfo^, int = 0);
    void fillConnectionInfo(const Ice::IPConnectionInfoPtr&) const;

    friend class TcpConnector;
    friend class TcpAcceptor;

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
