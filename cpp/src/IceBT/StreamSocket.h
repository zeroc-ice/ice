// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_STREAM_SOCKET_H
#define ICE_BT_STREAM_SOCKET_H

#include "../Ice/Network.h"
#include "BTBufSize.h"
#include "Config.h"
#include "Ice/Buffer.h"
#include "InstanceF.h"

namespace IceBT
{
    class StreamSocket : public IceInternal::NativeInfo
    {
    public:
        // For an incoming connection: wraps the accepted socket and configures it with the given buffer sizes.
        StreamSocket(InstancePtr, SOCKET, const BTBufSize&);

        // For an outgoing connection: the socket is set with setFd once the connection is established.
        explicit StreamSocket(InstancePtr);

        ~StreamSocket() override;

        void setBufferSize(SOCKET, int rcvSize, int sndSize);

        IceInternal::SocketOperation read(IceInternal::Buffer&);
        IceInternal::SocketOperation write(IceInternal::Buffer&);

        ssize_t read(char*, size_t);
        ssize_t write(const char*, size_t);

        void close();
        [[nodiscard]] const std::string& toString() const;

        // Sets and configures the socket of an outgoing connection once the connection is established.
        void setFd(SOCKET);

    private:
        const InstancePtr _instance;
        std::string _desc;
    };
    using StreamSocketPtr = std::shared_ptr<StreamSocket>;
}

#endif
