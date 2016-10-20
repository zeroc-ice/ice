// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_TRANSCEIVER_H
#define ICE_BT_TRANSCEIVER_H

#include <IceBT/InstanceF.h>
#include <IceBT/Engine.h>
#include <IceBT/StreamSocket.h>

#include <Ice/Transceiver.h>
#include <IceUtil/UniquePtr.h>

namespace IceBT
{

class ConnectorI;
class AcceptorI;

class TransceiverI : public IceInternal::Transceiver
{
public:

    virtual IceInternal::NativeInfoPtr getNativeInfo();

    virtual IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&);
    virtual IceInternal::SocketOperation closing(bool, const Ice::LocalException&);
    virtual void close();
    virtual IceInternal::SocketOperation write(IceInternal::Buffer&);
    virtual IceInternal::SocketOperation read(IceInternal::Buffer&);
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const IceInternal::Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

private:

    TransceiverI(const InstancePtr&, const StreamSocketPtr&, const ConnectionPtr&, const std::string&);
    TransceiverI(const InstancePtr&, const std::string&, const std::string&);
    virtual ~TransceiverI();

    friend class ConnectorI;
    friend class AcceptorI;

    const InstancePtr _instance;
    StreamSocketPtr _stream;
    ConnectionPtr _connection;
    std::string _addr;
    std::string _uuid;
    bool _needConnect;
    IceUtil::UniquePtr<Ice::Exception> _exception;
    IceUtil::Monitor<IceUtil::Mutex> _lock;

    void connectCompleted(int, const ConnectionPtr&);
    void connectFailed(const Ice::LocalException&);

    class ConnectCallbackI : public ConnectCallback
    {
    public:

        ConnectCallbackI(const TransceiverIPtr& transceiver) :
            _transceiver(transceiver)
        {
        }

        virtual void completed(int fd, const ConnectionPtr& conn)
        {
            _transceiver->connectCompleted(fd, conn);
        }

        virtual void failed(const Ice::LocalException& ex)
        {
            _transceiver->connectFailed(ex);
        }

    private:

        TransceiverIPtr _transceiver;
    };
    friend class ConnectCallbackI;
};

}

#endif
