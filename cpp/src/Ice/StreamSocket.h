// Copyright (c) ZeroC, Inc.

#ifndef ICE_STREAM_SOCKET_H
#define ICE_STREAM_SOCKET_H

#include "Ice/Buffer.h"
#include "Network.h"
#include "ProtocolInstanceF.h"

#include <memory>

namespace IceInternal
{
    class ICE_API StreamSocket : public NativeInfo
    {
    public:
        StreamSocket(ProtocolInstancePtr, const NetworkProxyPtr&, const Address&, const Address&);
        StreamSocket(ProtocolInstancePtr, SOCKET);
        ~StreamSocket() override;

        SocketOperation connect(Buffer&, Buffer&);
        bool isConnected();

        void setBufferSize(int rcvSize, int sndSize);

        SocketOperation read(Buffer&);
        SocketOperation write(Buffer&);

        ssize_t read(char*, size_t);
        ssize_t write(const char*, size_t);

#if defined(ICE_USE_IOCP)
        AsyncInfo* getAsyncInfo(SocketOperation);
        bool startWrite(Buffer&);
        void finishWrite(Buffer&);
        void startRead(Buffer&);
        void finishRead(Buffer&);
#endif

        void close();
        [[nodiscard]] const std::string& toString() const;

    private:
        void init();

        enum State
        {
            StateNeedConnect,
            StateConnectPending,
            StateProxyWrite,
            StateProxyRead,
            StateProxyConnected,
            StateConnected
        };
        [[nodiscard]] State toState(SocketOperation) const;

        const ProtocolInstancePtr _instance;
        const NetworkProxyPtr _proxy;
        const Address _addr;
        const Address _sourceAddr;

        State _state;
        std::string _desc;

#if defined(ICE_USE_IOCP)
        AsyncInfo _read;
        AsyncInfo _write;
#endif
    };
    using StreamSocketPtr = std::shared_ptr<StreamSocket>;
}

#endif
