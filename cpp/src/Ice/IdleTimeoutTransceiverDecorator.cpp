//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IdleTimeoutTransceiverDecorator.h"
#include "Ice/Buffer.h"

#include <chrono>
#include <iostream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

SocketOperation
IdleTimeoutTransceiverDecorator::initialize(Buffer& readBuffer, Buffer& writeBuffer)
{
    cerr << "initialize " << chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()) % 10000 << endl;
    SocketOperation op = _decoratee->initialize(readBuffer, writeBuffer);

    if (op == SocketOperationNone) // connected
    {
        if (_writeIdleTimeout != chrono::milliseconds::zero())
        {
            rescheduleHeartbeat();
        }

        if (_readIdleTimeout != chrono::milliseconds::zero())
        {
            _timer->schedule(_abortConnectionTimerTask, _readIdleTimeout);
        }
    }

    return op;
}

IdleTimeoutTransceiverDecorator::~IdleTimeoutTransceiverDecorator()
{
    if (_heartbeatTimerTask)
    {
        _timer->cancel(_heartbeatTimerTask);
    }
    if (_abortConnectionTimerTask)
    {
        _timer->cancel(_abortConnectionTimerTask);
    }
}

void
IdleTimeoutTransceiverDecorator::close()
{
    _timer->cancel(_heartbeatTimerTask);
    _timer->cancel(_abortConnectionTimerTask);
    _decoratee->close();
}

SocketOperation
IdleTimeoutTransceiverDecorator::write(Buffer& buf)
{
    cerr << "write " << chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch())  % 10000  << endl;
    Buffer::Container::iterator start = buf.i;
    SocketOperation op = _decoratee->write(buf);
    if (buf.i != start)
    {
        if (_writeIdleTimeout != chrono::milliseconds::zero())
        {
            // We've sent at least one byte, reschedule the heartbeat.
            rescheduleHeartbeat();
        }
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
    if (buf.i != start && _writeIdleTimeout != chrono::milliseconds::zero())
    {
        // We've sent at least one byte, reschedule the heartbeat.
        rescheduleHeartbeat();
    }
}

void
IdleTimeoutTransceiverDecorator::startRead(Buffer& buf)
{
    _decoratee->startRead(buf);
}

void
IdleTimeoutTransceiverDecorator::finishRead(Buffer& buf)
{
    Buffer::Container::iterator start = buf.i;
    _decoratee->finishRead(buf);
    if (_readIdleTimeout != chrono::milliseconds::zero())
    {
        if (buf.i != start)
        {
            // We've read something, reschedule the idle check.
            _timer->schedule(_abortConnectionTimerTask, _readIdleTimeout, true);
        }
        // else don't touch the existing idle check.
    }
}
#endif

// TODO: this naive implementation is not correct as it aborts a "ready" connection that was not read in a timely
// fashion.
SocketOperation
IdleTimeoutTransceiverDecorator::read(Buffer& buf)
{
    cerr << "read " << chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()) % 10000 << endl;
    Buffer::Container::iterator start = buf.i;
    SocketOperation op = _decoratee->read(buf);
    if (_readIdleTimeout != chrono::milliseconds::zero())
    {
        if (buf.i != start)
        {
            // We've read something, reschedule the idle check.
            _timer->schedule(_abortConnectionTimerTask, _readIdleTimeout, true);
        }
        // else don't touch the existing idle check.
    }
    return op;
}

void
IdleTimeoutTransceiverDecorator::rescheduleHeartbeat()
{
    _timer->schedule(_heartbeatTimerTask, _writeIdleTimeout / 2, true);
}
