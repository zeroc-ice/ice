//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IdleTimeoutTransceiverDecorator.h"
#include "Ice/Buffer.h"

#include <chrono>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    class HeartbeatTimerTask final : public IceUtil::TimerTask
    {
    public:
        HeartbeatTimerTask(const ConnectionIPtr& connection) : _connection(connection) {}

        void runTimerTask() final
        {
            if (auto connection = _connection.lock())
            {
                connection->sendHeartbeat();
            }
            // else nothing to do, the connection is already gone.
        }

    private:
        const std::weak_ptr<ConnectionI> _connection;
    };

    class IdleCheckTimerTask final : public IceUtil::TimerTask, public std::enable_shared_from_this<IdleCheckTimerTask>
    {
    public:
        IdleCheckTimerTask(
            const ConnectionIPtr& connection,
            const chrono::milliseconds& idleTimeout,
            const IceUtil::TimerPtr& timer)
            : _connection(connection),
              _idleTimeout(idleTimeout),
              _timer(timer)
        {
        }

        void runTimerTask() final
        {
            if (auto connection = _connection.lock())
            {
                if (connection->idleCheck())
                {
                    // Reschedule myself.
                    // We set cancelPrevious to true in the unlikely event some other thread is concurrently reading and
                    // already rescheduled this task.
                    _timer->schedule(shared_from_this(), _idleTimeout, true);
                }
                // else the connection was aborted by the idle check or is no longer active for some other reason.
            }
            // else nothing to do, the connection is already gone.
        }

    private:
        const std::weak_ptr<ConnectionI> _connection;
        const chrono::milliseconds _idleTimeout;
        const IceUtil::TimerPtr _timer;
    };
}

void
IdleTimeoutTransceiverDecorator::decoratorInit(const ConnectionIPtr& connection)
{
    _heartbeatTimerTask = make_shared<HeartbeatTimerTask>(connection);
    if (_enableIdleCheck)
    {
        _idleCheckTimerTask = make_shared<IdleCheckTimerTask>(connection, _idleTimeout, _timer);
    }
}

SocketOperation
IdleTimeoutTransceiverDecorator::initialize(Buffer& readBuffer, Buffer& writeBuffer)
{
    SocketOperation op = _decoratee->initialize(readBuffer, writeBuffer);

    if (op == SocketOperationNone) // connected
    {
        rescheduleHeartbeat();
        if (_enableIdleCheck)
        {
            // Reschedule because with SSL, the connection is connected after a read.
            _timer->schedule(_idleCheckTimerTask, _idleTimeout, true);
        }
    }

    return op;
}

IdleTimeoutTransceiverDecorator::~IdleTimeoutTransceiverDecorator()
{
    // It's possible but unlikely init was not called so initialize the timer tasks.
    if (_heartbeatTimerTask)
    {
        _timer->cancel(_heartbeatTimerTask);
    }
    if (_idleCheckTimerTask)
    {
        _timer->cancel(_idleCheckTimerTask);
    }
}

void
IdleTimeoutTransceiverDecorator::close()
{
    _timer->cancel(_heartbeatTimerTask);
    if (_enableIdleCheck)
    {
        _timer->cancel(_idleCheckTimerTask);
    }
    _decoratee->close();
}

SocketOperation
IdleTimeoutTransceiverDecorator::write(Buffer& buf)
{
    Buffer::Container::iterator start = buf.i;
    SocketOperation op = _decoratee->write(buf);
    if (buf.i != start)
    {
        // We've sent at least one byte, reschedule the heartbeat.
        rescheduleHeartbeat();
    }
    return op;
}

#if defined(ICE_USE_IOCP)
bool
IdleTimeoutTransceiverDecorator::startWrite(Buffer& buf)
{
    // TODO: should we check buf? the return value? The tracing code in ConnectionI doesn't. It's also not clear if the
    // main startWrite (StreamSocket::startWrite) actually modifies buf.
    return _decoratee->startWrite(buf);
}

void
IdleTimeoutTransceiverDecorator::finishWrite(Buffer& buf)
{
    Buffer::Container::iterator start = buf.i;
    _decoratee->finishWrite(buf);
    if (buf.i != start)
    {
        // We've sent at least one byte, reschedule the heartbeat.
        rescheduleHeartbeat();
    }
}

void
IdleTimeoutTransceiverDecorator::startRead(Buffer& buf)
{
    if (_enableIdleCheck)
    {
        // We reschedule the idle check as soon as possible to reduce the chances it kicks in while we're reading.
        _timer->schedule(_idleCheckTimerTask, _idleTimeout, true);
    }

    _decoratee->startRead(buf);
}

#endif

SocketOperation
IdleTimeoutTransceiverDecorator::read(Buffer& buf)
{
    if (_enableIdleCheck)
    {
        // We reschedule the idle check as soon as possible to reduce the chances it kicks in while we're reading.
        _timer->schedule(_idleCheckTimerTask, _idleTimeout, true);
    }
    return _decoratee->read(buf);
}

void
IdleTimeoutTransceiverDecorator::rescheduleHeartbeat()
{
    _timer->schedule(_heartbeatTimerTask, _idleTimeout / 2, true);
}
