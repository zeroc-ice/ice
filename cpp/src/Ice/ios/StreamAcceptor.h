//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_STREAM_ACCEPTOR_H
#define ICE_STREAM_ACCEPTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/Acceptor.h>

namespace IceObjC
{

    class StreamEndpointI;
    using StreamEndpointIPtr = std::shared_ptr<StreamEndpointI>;

    class StreamAcceptor final : public IceInternal::Acceptor,
                                 public IceInternal::NativeInfo,
                                 public std::enable_shared_from_this<StreamAcceptor>
    {
    public:
        StreamAcceptor(const StreamEndpointIPtr&, const InstancePtr&, const std::string&, int);
        ~StreamAcceptor();

        IceInternal::NativeInfoPtr getNativeInfo() final;
        void close() final;
        IceInternal::EndpointIPtr listen() final;
        IceInternal::TransceiverPtr accept() final;
        std::string protocol() const final;
        std::string toString() const final;
        std::string toDetailedString() const final;

        int effectivePort() const;

    private:
        friend class StreamEndpointI;

        StreamEndpointIPtr _endpoint;
        InstancePtr _instance;
        int _backlog;
        IceInternal::Address _addr;
    };

}

#endif
