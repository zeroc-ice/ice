// Copyright (c) ZeroC, Inc.

#ifndef ICE_NETWORK_FRAMEWORK_TRANSCEIVER_H
#define ICE_NETWORK_FRAMEWORK_TRANSCEIVER_H

#include "../Network.h"
#include "../ProtocolInstanceF.h"
#include "../Transceiver.h"

#include <Network/Network.h>
#include <dispatch/dispatch.h>

#include <atomic>
#include <mutex>
#include <vector>

namespace IceInternal
{
    //
    // Shared async I/O state for Network.framework completion blocks.
    // These structs are held by shared_ptr to ensure they outlive the transceiver
    // if a completion block fires after the transceiver is destroyed.
    //
    struct ReadState
    {
        std::mutex mutex;
        std::vector<std::byte> data;
        int error{0};
    };

    struct WriteState
    {
        std::mutex mutex;
        size_t count{0};
        int error{0};
    };

    struct ConnectState
    {
        std::atomic<bool> connected{false};
        std::atomic<int> error{0};
    };

    class NetworkFrameworkTransceiver final : public Transceiver
    {
    public:
        NetworkFrameworkTransceiver(ProtocolInstancePtr, nw_connection_t);
        ~NetworkFrameworkTransceiver();

        NativeInfoPtr getNativeInfo() final;

        SocketOperation initialize(Buffer&, Buffer&) final;
        SocketOperation closing(bool, std::exception_ptr) final;

        void close() final;
        SocketOperation write(Buffer&) final;
        SocketOperation read(Buffer&) final;

        bool startWrite(Buffer&) final;
        void finishWrite(Buffer&) final;
        void startRead(Buffer&) final;
        void finishRead(Buffer&) final;

        [[nodiscard]] std::string protocol() const final;
        [[nodiscard]] std::string toString() const final;
        [[nodiscard]] std::string toDetailedString() const final;
        [[nodiscard]] Ice::ConnectionInfoPtr
        getInfo(bool incoming, std::string adapterName, std::string connectionId) const final;
        void checkSendSize(const Buffer&) final;
        void setBufferSize(int rcvSize, int sndSize) final;

    private:
        enum State
        {
            StateNeedsConnect,
            StateConnectPending,
            StateConnected
        };

        const ProtocolInstancePtr _instance;
        NativeInfoPtr _nativeInfo;

        nw_connection_t _connection;
        dispatch_queue_t _dispatchQueue;
        State _state{StateNeedsConnect};

        // Shared async state — captured by completion blocks via shared_ptr.
        std::shared_ptr<ConnectState> _connectState;
        std::shared_ptr<ReadState> _readState;
        std::shared_ptr<WriteState> _writeState;

        std::string _desc;
    };
}

#endif
