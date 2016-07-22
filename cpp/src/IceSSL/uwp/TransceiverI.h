// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_UWP_TRANSCEIVER_I_H
#define ICE_SSL_UWP_TRANSCEIVER_I_H

#include <Ice/ProtocolInstanceF.h>
#include <Ice/Transceiver.h>
#include <Ice/Network.h>
#include <Ice/WSTransceiver.h>

namespace IceSSL
{

class ConnectorI;
class AcceptorI;

class TransceiverI : public IceInternal::Transceiver,
                     public IceInternal::NativeInfo,
                     public IceInternal::WSTransceiverDelegate
{
    enum State
    {
        StateNeedConnect,
        StateConnectPending,
        StateConnected
    };

public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();
    virtual void setCompletedHandler(IceInternal::SocketOperationCompletedHandler^);

    virtual IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&);

    virtual IceInternal::SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual IceInternal::SocketOperation write(IceInternal::Buffer&);
    virtual IceInternal::SocketOperation read(IceInternal::Buffer&);

    virtual bool startWrite(IceInternal::Buffer&);
    virtual void finishWrite(IceInternal::Buffer&);
    virtual void startRead(IceInternal::Buffer&);
    virtual void finishRead(IceInternal::Buffer&);

    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual Ice::ConnectionInfoPtr getWSInfo(const Ice::HeaderDict&) const;
    virtual void checkSendSize(const IceInternal::Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

private:

    TransceiverI(const IceInternal::ProtocolInstancePtr&, SOCKET, bool);
    virtual ~TransceiverI();

    void connect(const IceInternal::Address&);
    bool checkIfErrorOrCompleted(IceInternal::SocketOperation, Windows::Foundation::IAsyncInfo^, int = 0);
    void fillConnectionInfo(const Ice::IPConnectionInfoPtr&) const;

    friend class ConnectorI;
    friend class AcceptorI;

    const IceInternal::ProtocolInstancePtr _instance;

    State _state;
    std::string _desc;
    bool _verified;
    IceInternal::Address _connectAddr;

    IceInternal::AsyncInfo _read;
    IceInternal::AsyncInfo _write;
    int _maxSendPacketSize;
    int _maxReceivePacketSize;

    Windows::Storage::Streams::DataReader^ _reader;
    Windows::Storage::Streams::DataWriter^ _writer;

    Windows::Foundation::AsyncOperationCompletedHandler<unsigned int>^ _readOperationCompletedHandler;
    Windows::Foundation::AsyncOperationCompletedHandler<unsigned int>^ _writeOperationCompletedHandler;
};

}

#endif
