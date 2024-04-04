//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IdleTimeoutTransceiverDecorator.h"
#include "Ice/Buffer.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

SocketOperation
IdleTimeoutTransceiverDecorator::initialize(Buffer& readBuffer, Buffer& writeBuffer)
{
    SocketOperation op = _decoratee->initialize(readBuffer, writeBuffer);

    if (op == SocketOperationNone) // connected
    {
        rescheduleKeepAlive();
    }
    return op;
}

IdleTimeoutTransceiverDecorator::~IdleTimeoutTransceiverDecorator() { _timer->cancel(_keepAliveAction); }

void
IdleTimeoutTransceiverDecorator::close()
{
    _timer->cancel(_keepAliveAction);
    _decoratee->close();
}

SocketOperation
IdleTimeoutTransceiverDecorator::write(Buffer& buf)
{
    Buffer::Container::iterator start = buf.i;
    SocketOperation op = _decoratee->write(buf);
    if (buf.i != start)
    {
        // We've sent at least one byte, reschedule the keep alive.
        rescheduleKeepAlive();
    }
    return op;
}

#if defined(ICE_USE_IOCP)
bool
IdleTimeoutTransceiverDecorator::startWrite(Buffer& buf)
{
    // TODO: do we always call finishWrite? Should we check buf? The tracing code in ConnectionI doesn't.
    return _decoratee->startWrite(buf);
}

void
IdleTimeoutTransceiverDecorator::finishWrite(Buffer& buf)
{
    Buffer::Container::iterator start = buf.i;
    _decoratee->finishWrite(buf);
    if (buf.i != start)
    {
        // We've sent at least one byte, reschedule the keep alive.
        rescheduleKeepAlive();
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
    _decoratee->finishRead(buf);
}
#endif

SocketOperation
IdleTimeoutTransceiverDecorator::read(Buffer& buf)
{
    // Don't do anything for now.
    return _decoratee->read(buf);
}

void
IdleTimeoutTransceiverDecorator::rescheduleKeepAlive()
{
    // Each connection must have its own keepAliveAction, otherwise this code doesn't work.
    _timer->schedule(_keepAliveAction, _writeIdleTimeout / 2, true);
}
