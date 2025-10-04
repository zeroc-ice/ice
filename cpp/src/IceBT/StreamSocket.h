// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_STREAM_SOCKET_H
#define ICE_BT_STREAM_SOCKET_H

#include "../Ice/Network.h"
#include "Config.h"
#include "Ice/Buffer.h"
#include "InstanceF.h"

namespace IceBT
{
    class StreamSocket : public IceInternal::NativeInfo
    {
    public:
        StreamSocket(InstancePtr, SOCKET);
        ~StreamSocket() override;

        void setBufferSize(SOCKET, int rcvSize, int sndSize);

        IceInternal::SocketOperation read(IceInternal::Buffer&);
        IceInternal::SocketOperation write(IceInternal::Buffer&);

        ssize_t read(char*, size_t);
        ssize_t write(const char*, size_t);

        void close();
        [[nodiscard]] const std::string& toString() const;

        void setFd(SOCKET);

    private:
        void init(SOCKET);

        const InstancePtr _instance;
        SocketAddress _addr;
        std::string _desc;
    };
    using StreamSocketPtr = std::shared_ptr<StreamSocket>;
}

#endif
