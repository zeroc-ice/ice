// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_ACCEPTOR_I_H
#define ICE_BT_ACCEPTOR_I_H

#include "../Ice/Acceptor.h"
#include "../Ice/TransceiverF.h"
#include "Config.h"
#include "EngineF.h"
#include "InstanceF.h"

#include <mutex>
#include <stack>

namespace IceBT
{
    class AcceptorI final : public IceInternal::Acceptor,
                            public IceInternal::NativeInfo,
                            public std::enable_shared_from_this<AcceptorI>
    {
    public:
        AcceptorI(EndpointIPtr, InstancePtr, std::string, std::string, std::string, std::string, int);
        ~AcceptorI() final;
        IceInternal::NativeInfoPtr getNativeInfo() final;

        void close() final;
        IceInternal::EndpointIPtr listen() final;
        IceInternal::TransceiverPtr accept() final;
        std::string protocol() const final;
        std::string toString() const final;
        std::string toDetailedString() const final;

        int effectiveChannel() const;

        void newConnection(int);

    private:
        EndpointIPtr _endpoint;
        const InstancePtr _instance;
        const std::string _adapterName;
        const std::string _addr;
        const std::string _uuid;
        const std::string _name;
        const int _channel;
        std::string _path;

        std::mutex _mutex;
        std::stack<IceInternal::TransceiverPtr> _transceivers;
    };
}

#endif
