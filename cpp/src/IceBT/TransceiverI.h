// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_TRANSCEIVER_H
#define ICE_BT_TRANSCEIVER_H

#include "../Ice/Transceiver.h"
#include "Engine.h"
#include "InstanceF.h"
#include "StreamSocket.h"

namespace IceBT
{
    class ConnectorI;
    class AcceptorI;

    class TransceiverI final : public IceInternal::Transceiver, public std::enable_shared_from_this<TransceiverI>
    {
    public:
        TransceiverI(InstancePtr, StreamSocketPtr, ConnectionPtr, std::string);
        TransceiverI(InstancePtr, std::string, std::string);
        ~TransceiverI();
        IceInternal::NativeInfoPtr getNativeInfo() final;

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
        void setBufferSize(int rcvSize, int sndSize) final;

    private:
        const InstancePtr _instance;
        StreamSocketPtr _stream;
        ConnectionPtr _connection;
        std::string _addr;
        std::string _uuid;
        bool _needConnect;
        std::exception_ptr _exception;
        std::mutex _mutex;
        void connectCompleted(int, const ConnectionPtr&);
        void connectFailed(std::exception_ptr);

        class ConnectCallbackI final : public ConnectCallback
        {
        public:
            ConnectCallbackI(TransceiverIPtr transceiver) : _transceiver(std::move(transceiver)) {}

            void completed(int fd, const ConnectionPtr& conn) final { _transceiver->connectCompleted(fd, conn); }

            void failed(std::exception_ptr ex) final { _transceiver->connectFailed(ex); }

        private:
            TransceiverIPtr _transceiver;
        };
        friend class ConnectCallbackI;
    };
}

#endif
