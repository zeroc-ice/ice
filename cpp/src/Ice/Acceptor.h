// Copyright (c) ZeroC, Inc.

#ifndef ICE_ACCEPTOR_H
#define ICE_ACCEPTOR_H

#include "AcceptorF.h"
#include "EndpointIF.h"
#include "Network.h"
#include "TransceiverF.h"

#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace IceInternal
{
    class Acceptor
    {
    public:
        virtual ~Acceptor() = default;

        virtual NativeInfoPtr getNativeInfo() = 0;
        virtual void close() = 0;
        virtual EndpointIPtr listen() = 0;
#if defined(ICE_USE_IOCP)
        virtual void startAccept() = 0;
        virtual void finishAccept() = 0;
#endif
        virtual TransceiverPtr accept() = 0;
        [[nodiscard]] virtual std::string protocol() const = 0;
        [[nodiscard]] virtual std::string toString() const = 0;
        [[nodiscard]] virtual std::string toDetailedString() const = 0;
    };
}

#ifdef __clang__
#    pragma clang diagnostic pop
#endif

#endif
