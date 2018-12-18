// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TCP_TRANSCEIVER_H
#define ICE_TCP_TRANSCEIVER_H

#include <Ice/ProtocolInstanceF.h>
#include <Ice/Transceiver.h>
#include <Ice/Network.h>
#include <Ice/StreamSocket.h>

namespace IceInternal
{

class TcpConnector;
class TcpAcceptor;

class TcpTransceiver : public Transceiver
{
public:

    virtual NativeInfoPtr getNativeInfo();

    virtual SocketOperation initialize(Buffer&, Buffer&);
    virtual SocketOperation closing(bool, const Ice::LocalException&);

    virtual void close();
    virtual SocketOperation write(Buffer&);
    virtual SocketOperation read(Buffer&);
#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
    virtual bool startWrite(Buffer&);
    virtual void finishWrite(Buffer&);
    virtual void startRead(Buffer&);
    virtual void finishRead(Buffer&);
#endif
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;
    virtual Ice::ConnectionInfoPtr getInfo() const;
    virtual void checkSendSize(const Buffer&);
    virtual void setBufferSize(int rcvSize, int sndSize);

private:

    TcpTransceiver(const ProtocolInstancePtr&, const StreamSocketPtr&);
    virtual ~TcpTransceiver();

    friend class TcpConnector;
    friend class TcpAcceptor;

    const ProtocolInstancePtr _instance;
    const StreamSocketPtr _stream;
};

}

#endif
