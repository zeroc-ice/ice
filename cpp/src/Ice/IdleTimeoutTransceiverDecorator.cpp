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
        IdleCheckTimerTask(const ConnectionIPtr& connection, const chrono::milliseconds& idleTimeout)
            : _connection(connection),
              _idleTimeout(idleTimeout)
        {
        }

        void runTimerTask() final
        {
            if (auto connection = _connection.lock())
            {
                connection->idleCheck(shared_from_this(), _idleTimeout);
            }
            // else nothing to do, the connection is already gone.
        }

    private:
        const std::weak_ptr<ConnectionI> _connection;
        const chrono::milliseconds _idleTimeout;
    };
}

void
IdleTimeoutTransceiverDecorator::decoratorInit(const ConnectionIPtr& connection)
{
    _heartbeatTimerTask = make_shared<HeartbeatTimerTask>(connection);
    if (_enableIdleCheck)
    {
        _idleCheckTimerTask = make_shared<IdleCheckTimerTask>(connection, _idleTimeout);
    }
}

SocketOperation
IdleTimeoutTransceiverDecorator::initialize(Buffer& readBuffer, Buffer& writeBuffer)
{
    SocketOperation op = _decoratee->initialize(readBuffer, writeBuffer);

    if (op == SocketOperationNone) // connected
    {
        // cancelPrevious = true because Ice often writes to a client connection before it's connected.
        _timer->schedule(_heartbeatTimerTask, _idleTimeout / 2, true);
        if (_enableIdleCheck)
        {
            // cancelPrevious = true because with SSL, the connection is connected after a read.
            _timer->schedule(_idleCheckTimerTask, _idleTimeout, true);
        }
    }

    return op;
}

IdleTimeoutTransceiverDecorator::~IdleTimeoutTransceiverDecorator()
{
    // If we destroy this object before calling decoratorInit(), _heartbeatTimerTask and _idleCheckTimerTask will be
    // null.
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
    // We're about to write something and we don't want to send a concurrent heartbeat.
    _timer->cancel(_heartbeatTimerTask);

    SocketOperation op = _decoratee->write(buf);
    if (op == SocketOperationNone)
    {
        _timer->schedule(_heartbeatTimerTask, _idleTimeout / 2);
    }

    return op;
}

#if defined(ICE_USE_IOCP)
bool
IdleTimeoutTransceiverDecorator::startWrite(Buffer& buf)
{
    // We're about to write something and we don't want to send a concurrent heartbeat.
    _timer->cancel(_heartbeatTimerTask);

    return _decoratee->startWrite(buf);

    // TODO: should we schedule the next heartbeat when the return value is true? Or do we always call finishWrite?
}

void
IdleTimeoutTransceiverDecorator::finishWrite(Buffer& buf)
{
    _decoratee->finishWrite(buf);

    // Schedule a heartbeat after writing all the data.
    if (buf.i == buf.b.end())
    {
        _timer->schedule(_heartbeatTimerTask, _idleTimeout / 2);
    }
}

void
IdleTimeoutTransceiverDecorator::startRead(Buffer& buf)
{
    // We always call finishRead or read to actually read the data.
    _decoratee->startRead(buf);
}

void
IdleTimeoutTransceiverDecorator::finishRead(Buffer& buf)
{
    if (_enableIdleCheck)
    {
        // We don't want the idle check to run while we're reading, so we reschedule it before reading.
        _timer->schedule(_idleCheckTimerTask, _idleTimeout, true);
    }

    _decoratee->finishRead(buf);
}

#endif

SocketOperation
IdleTimeoutTransceiverDecorator::read(Buffer& buf)
{
    if (_enableIdleCheck)
    {
        // We don't want the idle check to run while we're reading, so we reschedule it before reading.
        _timer->schedule(_idleCheckTimerTask, _idleTimeout, true);
    }
    return _decoratee->read(buf);
}
