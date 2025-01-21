// Copyright (c) ZeroC, Inc.

#ifndef ICE_STREAM_TRANSCEIVER_H
#define ICE_STREAM_TRANSCEIVER_H

#include "Ice/Config.h"

#if TARGET_OS_IPHONE != 0

#    include "../Network.h"
#    include "../Selector.h"
#    include "../UniqueRef.h"
#    include "../WSTransceiver.h"

struct __CFError;
typedef struct __CFError* CFErrorRef;

struct __CFWriteStream;
typedef struct __CFWriteStream* CFWriteStreamRef;

struct __CFReadStream;
typedef struct __CFReadStream* CFReadStreamRef;

namespace IceObjC
{
    class Instance;
    using InstancePtr = std::shared_ptr<Instance>;

    class StreamTransceiver : public IceInternal::Transceiver,
                              public IceInternal::StreamNativeInfo,
                              public std::enable_shared_from_this<StreamTransceiver>
    {
        enum State
        {
            StateNeedConnect,
            StateConnectPending,
            StateConnected
        };

    public:
        StreamTransceiver(const InstancePtr&, CFReadStreamRef, CFWriteStreamRef, const std::string&, std::int32_t);
        StreamTransceiver(const InstancePtr&, CFReadStreamRef, CFWriteStreamRef, SOCKET);

        virtual ~StreamTransceiver();

        IceInternal::NativeInfoPtr getNativeInfo() final;

        void initStreams(IceInternal::SelectorReadyCallback*) final;
        IceInternal::SocketOperation registerWithRunLoop(IceInternal::SocketOperation) final;
        IceInternal::SocketOperation unregisterFromRunLoop(IceInternal::SocketOperation, bool) final;
        void closeStreams() final;

        IceInternal::SocketOperation initialize(IceInternal::Buffer&, IceInternal::Buffer&) final;
        IceInternal::SocketOperation closing(bool, std::exception_ptr) final;
        void close() final;

        IceInternal::SocketOperation write(IceInternal::Buffer&) final;
        IceInternal::SocketOperation read(IceInternal::Buffer&) final;

        std::string protocol() const final;
        std::string toString() const final;
        std::string toDetailedString() const final;
        Ice::ConnectionInfoPtr getInfo(bool incoming, std::string adapterName, std::string connectionId) const final;
        void checkSendSize(const IceInternal::Buffer&) final;
        void setBufferSize(int, int) final;

    private:
        void checkErrorStatus(CFWriteStreamRef, CFReadStreamRef, const char*, int);

        const InstancePtr _instance;
        const std::string _host;
        const std::int32_t _port;
        IceInternal::UniqueRef<CFReadStreamRef> _readStream;
        IceInternal::UniqueRef<CFWriteStreamRef> _writeStream;
        bool _readStreamRegistered;
        bool _writeStreamRegistered;
        bool _opening;

        std::mutex _mutex;
        bool _error;

        State _state;
        std::string _desc;
    };
}

#endif

#endif
