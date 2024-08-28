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
    class HeartbeatTimerTask final : public Ice::TimerTask
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
        const weak_ptr<ConnectionI> _connection;
    };

    class IdleCheckTimerTask final : public Ice::TimerTask
    {
    public:
        IdleCheckTimerTask(const ConnectionIPtr& connection, const chrono::seconds& idleTimeout)
            : _connection(connection),
              _idleTimeout(idleTimeout)
        {
        }

        void runTimerTask() final
        {
            if (auto connection = _connection.lock())
            {
                connection->idleCheck(_idleTimeout);
            }
            // else nothing to do, the connection is already gone.
        }

    private:
        const weak_ptr<ConnectionI> _connection;
        const chrono::seconds _idleTimeout;
    };
}

void
IdleTimeoutTransceiverDecorator::decoratorInit(const ConnectionIPtr& connection, bool enableIdleCheck)
{
    _heartbeatTimerTask = make_shared<HeartbeatTimerTask>(connection);
    if (enableIdleCheck)
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
        // reschedule because Ice often writes to a client connection before it's connected.
        _timer->reschedule(_heartbeatTimerTask, chrono::milliseconds(_idleTimeout) / 2);
    }

    return op;
}

IdleTimeoutTransceiverDecorator::~IdleTimeoutTransceiverDecorator()
{
    // Since ConnectionI is noexcept, decoratorInit always sets _heartbeatTimerTask.
    assert(_heartbeatTimerTask);
    _timer->cancel(_heartbeatTimerTask);
    disableIdleCheck();
}

void
IdleTimeoutTransceiverDecorator::close()
{
    _timer->cancel(_heartbeatTimerTask);
    disableIdleCheck();
    _decoratee->close();
}

SocketOperation
IdleTimeoutTransceiverDecorator::write(Buffer& buf)
{
    _timer->cancel(_heartbeatTimerTask);

    SocketOperation op = _decoratee->write(buf);
    if (op == SocketOperationNone) // write completed
    {
        _timer->schedule(_heartbeatTimerTask, chrono::milliseconds(_idleTimeout) / 2);
    }
    return op;
}

#if defined(ICE_USE_IOCP)
bool
IdleTimeoutTransceiverDecorator::startWrite(Buffer& buf)
{
    // startWrite is called right after write returns SocketOperationWrite. The logic in write is sufficient.
    return _decoratee->startWrite(buf);
}

void
IdleTimeoutTransceiverDecorator::finishWrite(Buffer& buf)
{
    _decoratee->finishWrite(buf);
    // We call write after finishWrite, so no need to do anything here.
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
    if (_idleCheckEnabled)
    {
        // We don't want the idle check to run while we're reading, so we reschedule it before reading.
        _timer->reschedule(_idleCheckTimerTask, _idleTimeout);
    }
    _decoratee->finishRead(buf);
}

#endif

SocketOperation
IdleTimeoutTransceiverDecorator::read(Buffer& buf)
{
    if (_idleCheckEnabled)
    {
        // We don't want the idle check to run while we're reading, so we reschedule it before reading.
        _timer->reschedule(_idleCheckTimerTask, _idleTimeout);
    }
    return _decoratee->read(buf);
}

void
IdleTimeoutTransceiverDecorator::enableIdleCheck()
{
    if (!_idleCheckEnabled && _idleCheckTimerTask)
    {
        _timer->reschedule(_idleCheckTimerTask, _idleTimeout);
        _idleCheckEnabled = true;
    }
}

void
IdleTimeoutTransceiverDecorator::disableIdleCheck()
{
    if (_idleCheckEnabled && _idleCheckTimerTask)
    {
        _timer->cancel(_idleCheckTimerTask);
        _idleCheckEnabled = false;
    }
}
