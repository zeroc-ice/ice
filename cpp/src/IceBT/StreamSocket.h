// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_STREAM_SOCKET_H
#define ICE_BT_STREAM_SOCKET_H

#include <IceBT/Config.h>
#include <IceBT/InstanceF.h>

#include <IceUtil/Shared.h>
#include <Ice/Network.h>
#include <Ice/Buffer.h>

namespace IceBT
{

class StreamSocket : public IceInternal::NativeInfo
{
public:

    StreamSocket(const InstancePtr&, SOCKET);
    virtual ~StreamSocket();

    size_t getSendPacketSize(size_t);
    size_t getRecvPacketSize(size_t);

    void setBufferSize(SOCKET, int rcvSize, int sndSize);

    IceInternal::SocketOperation read(IceInternal::Buffer&);
    IceInternal::SocketOperation write(IceInternal::Buffer&);

    ssize_t read(char*, size_t);
    ssize_t write(const char*, size_t);

    void close();
    const std::string& toString() const;

    void setFd(SOCKET);

private:

    void init(SOCKET);

    const InstancePtr _instance;
    SocketAddress _addr;
    std::string _desc;
};
typedef IceUtil::Handle<StreamSocket> StreamSocketPtr;

}

#endif
