//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IDLE_TIMEOUT_TRANSCEIVER_DECORATOR_H
#define ICE_IDLE_TIMEOUT_TRANSCEIVER_DECORATOR_H

#include "ConnectionI.h"
#include "IceUtil/Timer.h"
#include "Transceiver.h"

#include <cassert>

namespace IceInternal
{
    // Decorates Transceiver to send heartbeats and optionally detect when no byte is received/read for a while.
    // This decorator must not be applied on UDP connections.
    class IdleTimeoutTransceiverDecorator final : public Transceiver
    {
    public:
        IdleTimeoutTransceiverDecorator(
            const TransceiverPtr& decoratee,
            std::chrono::milliseconds idleTimeout,
            bool enableIdleCheck,
            const IceUtil::TimerPtr& timer)
            : _decoratee(decoratee),
              _idleTimeout(idleTimeout),
              _enableIdleCheck(enableIdleCheck),
              _timer(timer)
        {
            assert(_decoratee->protocol() != "udp");
        }

        ~IdleTimeoutTransceiverDecorator();

        // Set the timer tasks immediately after construction. Must be called only once.
        void decoratorInit(const Ice::ConnectionIPtr&);

        NativeInfoPtr getNativeInfo() final { return _decoratee->getNativeInfo(); }

        SocketOperation initialize(Buffer& readBuffer, Buffer& writeBuffer) final;

        SocketOperation closing(bool initiator, std::exception_ptr ex) final
        {
            return _decoratee->closing(initiator, ex);
        }

        void close() final;

        EndpointIPtr bind() final { return _decoratee->bind(); }

        SocketOperation write(Buffer&) final;
        SocketOperation read(Buffer&) final;

#if defined(ICE_USE_IOCP)
        bool startWrite(Buffer&) final;
        void finishWrite(Buffer&) final;
        void startRead(Buffer&) final;
        void finishRead(Buffer& buf) final { _decoratee->finishRead(buf); }
#endif

        bool hasDataAvailable() const noexcept final { return _decoratee->hasDataAvailable(); }
        std::string protocol() const final { return _decoratee->protocol(); }
        std::string toString() const final { return _decoratee->toString(); }
        std::string toDetailedString() const final { return _decoratee->toDetailedString(); }
        Ice::ConnectionInfoPtr getInfo() const final { return _decoratee->getInfo(); }
        void checkSendSize(const Buffer& buf) final { _decoratee->checkSendSize(buf); };
        void setBufferSize(int rcvSize, int sndSize) final { _decoratee->setBufferSize(rcvSize, sndSize); }

    private:
        void rescheduleHeartbeat();

        const TransceiverPtr _decoratee;
        const std::chrono::milliseconds _idleTimeout;
        const bool _enableIdleCheck;
        const IceUtil::TimerPtr _timer;

        // Set by decoratorInit
        IceUtil::TimerTaskPtr _heartbeatTimerTask;
        IceUtil::TimerTaskPtr _idleCheckTimerTask;
    };
}

#endif
