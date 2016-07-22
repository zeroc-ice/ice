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
#include <IceBT/StreamSocket.h>

#include <Ice/Transceiver.h>

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

    TransceiverI(const InstancePtr&, const StreamSocketPtr&, const std::string&);
    virtual ~TransceiverI();

    friend class ConnectorI;
    friend class AcceptorI;

    const InstancePtr _instance;
    const StreamSocketPtr _stream;
    const std::string _uuid;
};

}

#endif
