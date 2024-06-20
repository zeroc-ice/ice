//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_TRANSCEIVER_H
#define ICE_TRANSCEIVER_H

#include "EndpointIF.h"
#include "Ice/ConnectionF.h"
#include "Network.h"
#include "TransceiverF.h"

namespace IceInternal
{
    class Buffer;

    class ICE_API Transceiver
    {
    public:
        virtual NativeInfoPtr getNativeInfo() = 0;

        virtual SocketOperation initialize(Buffer&, Buffer&) = 0;
        virtual SocketOperation closing(bool, std::exception_ptr) = 0;

        virtual void close() = 0;
        virtual EndpointIPtr bind();
        virtual SocketOperation write(Buffer&) = 0;
        virtual SocketOperation read(Buffer&) = 0;

#if defined(ICE_USE_IOCP)
        virtual bool startWrite(Buffer&) = 0;
        virtual void finishWrite(Buffer&) = 0;
        virtual void startRead(Buffer&) = 0;
        virtual void finishRead(Buffer&) = 0;
#endif

        /// Checks if this transceiver is waiting to be read, typically because it has bytes readily available for
        /// reading.
        /// @return true if this transceiver is waiting to be read, false otherwise.
        /// @remark The caller must ensure the transceiver is not closed when calling this function.
        virtual bool isWaitingToBeRead() const noexcept = 0;

        virtual std::string protocol() const = 0;
        virtual std::string toString() const = 0;
        virtual std::string toDetailedString() const = 0;
        virtual Ice::ConnectionInfoPtr getInfo() const = 0;
        virtual void checkSendSize(const Buffer&) = 0;
        virtual void setBufferSize(int, int) = 0;
    };
}

#endif
