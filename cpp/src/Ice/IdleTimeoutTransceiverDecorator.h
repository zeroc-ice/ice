// Copyright (c) ZeroC, Inc.

#ifndef ICE_IDLE_TIMEOUT_TRANSCEIVER_DECORATOR_H
#define ICE_IDLE_TIMEOUT_TRANSCEIVER_DECORATOR_H

#include "ConnectionI.h"
#include "Timer.h"
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
            TransceiverPtr decoratee,
            const std::chrono::seconds& idleTimeout,
            IceInternal::TimerPtr timer)
            : _decoratee(std::move(decoratee)),
              _idleTimeout(idleTimeout),
              _timer(std::move(timer))
        {
            assert(_decoratee->protocol() != "udp");
        }

        ~IdleTimeoutTransceiverDecorator();

        // Set the timer tasks immediately after construction. Must be called only once.
        void decoratorInit(const Ice::ConnectionIPtr& connection, bool enableIdleCheck);

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
        void finishRead(Buffer& buf) final;
#endif

        [[nodiscard]] std::string protocol() const final { return _decoratee->protocol(); }
        [[nodiscard]] std::string toString() const final { return _decoratee->toString(); }
        [[nodiscard]] std::string toDetailedString() const final { return _decoratee->toDetailedString(); }

        [[nodiscard]] Ice::ConnectionInfoPtr
        getInfo(bool incoming, std::string adapterName, std::string connectionId) const final
        {
            return _decoratee->getInfo(incoming, std::move(adapterName), std::move(connectionId));
        }

        void checkSendSize(const Buffer& buf) final { _decoratee->checkSendSize(buf); };
        void setBufferSize(int rcvSize, int sndSize) final { _decoratee->setBufferSize(rcvSize, sndSize); }

        [[nodiscard]] bool idleCheckEnabled() const noexcept { return _idleCheckEnabled; }
        void enableIdleCheck();
        void disableIdleCheck();
        void scheduleHeartbeat();

    private:
        const TransceiverPtr _decoratee;
        const std::chrono::seconds _idleTimeout;
        const IceInternal::TimerPtr _timer;

        bool _idleCheckEnabled = false;

        // Set by decoratorInit
        IceInternal::TimerTaskPtr _heartbeatTimerTask;
        IceInternal::TimerTaskPtr _idleCheckTimerTask;
    };
}

#endif
