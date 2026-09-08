// Copyright (c) ZeroC, Inc.

#include "Ice/Config.h"

#if TARGET_OS_IPHONE != 0

#    include "../ProtocolInstance.h"
#    include "Ice/Buffer.h"
#    include "Ice/LocalExceptions.h"
#    include "iAPEndpointI.h"
#    include "iAPTransceiver.h"
#    include "iAPUtil.h"

#    import <Foundation/NSError.h>
#    import <Foundation/NSString.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using StreamState = IceObjC::iAPTransceiver::StreamState;

namespace
{
    // Converts the stream's error into the exception the connection reports.
    exception_ptr streamError(NSStream* stream, const char* file, int line)
    {
        NSStreamStatus status = [stream streamStatus];
        if (status == NSStreamStatusAtEnd || status == NSStreamStatusClosed)
        {
            return make_exception_ptr(ConnectionLostException{file, line});
        }

        NSError* err = [stream streamError];
        if (err == nil)
        {
            return make_exception_ptr(SocketException{file, line, "CFNetwork error", 0});
        }

        NSString* domain = [err domain];
        if ([domain compare:NSPOSIXErrorDomain] == NSOrderedSame)
        {
            errno = static_cast<int>([err code]);
            if (connectionRefused())
            {
                return make_exception_ptr(ConnectionRefusedException{file, line});
            }
            else if (connectFailed())
            {
                return make_exception_ptr(ConnectFailedException{file, line, getSocketErrno()});
            }
            else
            {
                return make_exception_ptr(SocketException{file, line, "CFNetwork error", getSocketErrno()});
            }
        }

        return make_exception_ptr(SocketException{
            file,
            line,
            "CFNetwork error in domain " + IceObjC::nsToString(domain) + ": " + to_string([err code])});
    }

    // The functions below run on the stream queue with the state mutex held.

    void completeConnect(StreamState& state)
    {
        if (state.connectPending)
        {
            state.connectPending = false;
            state.connectError = state.error;
            state.nativeInfo->completed(SocketOperationConnect);
        }
    }

    void doWrite(StreamState& state)
    {
        if (!state.writePending)
        {
            return;
        }

        if (!state.error)
        {
            if (![state.writeStream hasSpaceAvailable])
            {
                return; // Wait for NSStreamEventHasSpaceAvailable.
            }
            NSInteger ret = [state.writeStream write:reinterpret_cast<const UInt8*>(state.writeData)
                                           maxLength:state.writeSize];
            if (ret < 0)
            {
                state.error = streamError(state.writeStream, __FILE__, __LINE__);
            }
            else
            {
                state.writeCount = static_cast<size_t>(ret);
            }
        }

        state.writePending = false;
        state.writeError = state.error;
        state.nativeInfo->completed(SocketOperationWrite);
    }

    void doRead(StreamState& state)
    {
        if (!state.readPending)
        {
            return;
        }

        if (!state.error)
        {
            if (![state.readStream hasBytesAvailable])
            {
                return; // Wait for NSStreamEventHasBytesAvailable.
            }
            NSInteger ret = [state.readStream read:reinterpret_cast<UInt8*>(state.readData) maxLength:state.readSize];
            if (ret == 0)
            {
                state.error = make_exception_ptr(ConnectionLostException{__FILE__, __LINE__});
            }
            else if (ret < 0)
            {
                state.error = streamError(state.readStream, __FILE__, __LINE__);
            }
            else
            {
                state.readCount = static_cast<size_t>(ret);
            }
        }

        state.readPending = false;
        state.readError = state.error;
        state.nativeInfo->completed(SocketOperationRead);
    }

    // Completes the pending operations once state.error is set.
    void failPending(StreamState& state)
    {
        assert(state.error);
        completeConnect(state);
        doRead(state);
        doWrite(state);
    }
}

@interface iAPTransceiverCallback : NSObject <NSStreamDelegate>
{
@private

    std::shared_ptr<StreamState> state;
}
- (id)initWithState:(std::shared_ptr<StreamState>)st;
@end

@implementation iAPTransceiverCallback
- (id)initWithState:(std::shared_ptr<StreamState>)st
{
    self = [super init];
    if (!self)
    {
        return nil;
    }
    state = std::move(st);
    return self;
}

- (void)stream:(NSStream*)stream handleEvent:(NSStreamEvent)eventCode
{
    lock_guard lock(state->mutex);
    switch (eventCode)
    {
        case NSStreamEventOpenCompleted:
            if (stream == state->readStream)
            {
                state->readOpen = true;
            }
            else
            {
                state->writeOpen = true;
            }
            if (state->readOpen && state->writeOpen)
            {
                completeConnect(*state);
            }
            break;
        case NSStreamEventHasBytesAvailable:
            doRead(*state);
            break;
        case NSStreamEventHasSpaceAvailable:
            doWrite(*state);
            break;
        case NSStreamEventEndEncountered:
            if (!state->error)
            {
                state->error = make_exception_ptr(ConnectionLostException{__FILE__, __LINE__});
            }
            failPending(*state);
            break;
        default: // NSStreamEventErrorOccurred
            if (!state->error)
            {
                state->error = streamError(stream, __FILE__, __LINE__);
            }
            failPending(*state);
            break;
    }
}
@end

IceInternal::NativeInfoPtr
IceObjC::iAPTransceiver::getNativeInfo()
{
    return _nativeInfo;
}

SocketOperation
IceObjC::iAPTransceiver::initialize(Buffer& /*readBuffer*/, Buffer& /*writeBuffer*/)
{
    if (_state == StateNeedConnect)
    {
        _state = StateConnectPending;
        {
            lock_guard lock(_streamState->mutex);
            _streamState->connectPending = true;
        }

        // Open the streams on the stream queue, which also delivers their events (no run loop is involved).
        auto state = _streamState;
        iAPTransceiverCallback* callback = _callback;
        dispatch_queue_t queue = _queue;
        dispatch_async(_queue, ^{
          NSInputStream* readStream;
          NSOutputStream* writeStream;
          {
              lock_guard lock(state->mutex);
              if (state->error)
              {
                  failPending(*state); // Closed before the block ran.
                  return;
              }
              readStream = state->readStream;
              writeStream = state->writeStream;
          }
          [readStream setDelegate:callback];
          [writeStream setDelegate:callback];
          CFReadStreamSetDispatchQueue((CFReadStreamRef)readStream, queue);
          CFWriteStreamSetDispatchQueue((CFWriteStreamRef)writeStream, queue);
          [readStream open];
          [writeStream open];
        });
        return SocketOperationConnect;
    }

    if (_state == StateConnectPending)
    {
        lock_guard lock(_streamState->mutex);
        if (_streamState->connectError)
        {
            rethrow_exception(_streamState->connectError);
        }
        _state = StateConnected;
    }

    assert(_state == StateConnected);
    return SocketOperationNone;
}

SocketOperation
IceObjC::iAPTransceiver::closing(bool initiator, exception_ptr)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the connection. Otherwise, close immediately.
    return initiator ? SocketOperationRead : SocketOperationNone;
}

void
IceObjC::iAPTransceiver::close()
{
    // Close the streams on the stream queue, so that no event is delivered while closing, and complete the
    // pending operations: the thread pool waits for them.
    auto state = _streamState;
    dispatch_sync(_queue, ^{
      lock_guard lock(state->mutex);
      if (!state->error)
      {
          state->error = make_exception_ptr(ConnectionLostException{__FILE__, __LINE__});
      }
      [state->readStream setDelegate:nil];
      [state->writeStream setDelegate:nil];
      CFReadStreamSetDispatchQueue((CFReadStreamRef)state->readStream, nullptr);
      CFWriteStreamSetDispatchQueue((CFWriteStreamRef)state->writeStream, nullptr);
      [state->readStream close];
      [state->writeStream close];
      failPending(*state);
    });
}

SocketOperation
IceObjC::iAPTransceiver::write(Buffer& buf)
{
    // The streams are only used from the stream queue: writes are always asynchronous.
    return buf.i == buf.b.end() ? SocketOperationNone : SocketOperationWrite;
}

SocketOperation
IceObjC::iAPTransceiver::read(Buffer& buf)
{
    return buf.i == buf.b.end() ? SocketOperationNone : SocketOperationRead;
}

bool
IceObjC::iAPTransceiver::startWrite(Buffer& buf)
{
    if (_state == StateConnectPending)
    {
        // The connect is in progress, its completion is posted once the streams are open.
        return false;
    }

    assert(_state == StateConnected);
    assert(buf.i != buf.b.end());
    {
        lock_guard lock(_streamState->mutex);
        _streamState->writePending = true;
        _streamState->writeData = &*buf.i;
        _streamState->writeSize = static_cast<size_t>(buf.b.end() - buf.i);
        _streamState->writeCount = 0;
        _streamState->writeError = nullptr;
    }

    auto state = _streamState;
    dispatch_async(_queue, ^{
      lock_guard lock(state->mutex);
      doWrite(*state);
    });
    return true;
}

void
IceObjC::iAPTransceiver::finishWrite(Buffer& buf)
{
    if (_state < StateConnected)
    {
        return;
    }

    lock_guard lock(_streamState->mutex);
    if (_streamState->writeError)
    {
        rethrow_exception(_streamState->writeError);
    }
    buf.i += _streamState->writeCount;
}

void
IceObjC::iAPTransceiver::startRead(Buffer& buf)
{
    assert(_state == StateConnected);
    assert(buf.i != buf.b.end());
    {
        lock_guard lock(_streamState->mutex);
        _streamState->readPending = true;
        _streamState->readData = &*buf.i;
        _streamState->readSize = static_cast<size_t>(buf.b.end() - buf.i);
        _streamState->readCount = 0;
        _streamState->readError = nullptr;
    }

    auto state = _streamState;
    dispatch_async(_queue, ^{
      lock_guard lock(state->mutex);
      doRead(*state);
    });
}

void
IceObjC::iAPTransceiver::finishRead(Buffer& buf)
{
    lock_guard lock(_streamState->mutex);
    if (_streamState->readError)
    {
        rethrow_exception(_streamState->readError);
    }
    buf.i += _streamState->readCount;
}

string
IceObjC::iAPTransceiver::protocol() const
{
    return _instance->protocol();
}

string
IceObjC::iAPTransceiver::toString() const
{
    return _desc;
}

string
IceObjC::iAPTransceiver::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceObjC::iAPTransceiver::getInfo([[maybe_unused]] bool incoming, string adapterName, string connectionId) const
{
    assert(!incoming);

    if (_session == nil)
    {
        // Test-only transceiver constructed without an EASession; no accessory metadata is available.
        return make_shared<
            Ice::IAPConnectionInfo>(std::move(adapterName), std::move(connectionId), "", "", "", "", "", "");
    }

    return make_shared<Ice::IAPConnectionInfo>(
        std::move(adapterName),
        std::move(connectionId),
        nsToString(_session.accessory.name),
        nsToString(_session.accessory.manufacturer),
        nsToString(_session.accessory.modelNumber),
        nsToString(_session.accessory.firmwareRevision),
        nsToString(_session.accessory.hardwareRevision),
        nsToString(_session.protocolString));
}

void
IceObjC::iAPTransceiver::checkSendSize(const Buffer& /*buf*/)
{
}

void
IceObjC::iAPTransceiver::setBufferSize(int, int)
{
}

IceObjC::iAPTransceiver::iAPTransceiver(const ProtocolInstancePtr& instance, EASession* session)
    : _instance(instance),
#    if defined(__clang__) && !__has_feature(objc_arc)
      _session([session retain]),
#    else
      _session(session),
#    endif
      _readStream([session inputStream]),
      _writeStream([session outputStream]),
      _callback(nil),
      _queue(dispatch_queue_create("com.zeroc.ice.iap", DISPATCH_QUEUE_SERIAL)),
      _nativeInfo(make_shared<NativeInfo>(INVALID_SOCKET)),
      _streamState(make_shared<StreamState>()),
      _state(StateNeedConnect)
{
    _streamState->readStream = _readStream;
    _streamState->writeStream = _writeStream;
    _streamState->nativeInfo = _nativeInfo;
    _callback = [[iAPTransceiverCallback alloc] initWithState:_streamState];

    ostringstream os;
    os << "name = " << nsToString(session.accessory.name) << "\n";
    os << "protocol = " << nsToString(session.protocolString);
    _desc = os.str();
}

IceObjC::iAPTransceiver::iAPTransceiver(
    const ProtocolInstancePtr& instance,
    NSInputStream* readStream,
    NSOutputStream* writeStream,
    string desc)
    : _instance(instance),
      _session(nil),
      _readStream(readStream),
      _writeStream(writeStream),
      _callback(nil),
      _queue(dispatch_queue_create("com.zeroc.ice.iap", DISPATCH_QUEUE_SERIAL)),
      _nativeInfo(make_shared<NativeInfo>(INVALID_SOCKET)),
      _streamState(make_shared<StreamState>()),
      _state(StateNeedConnect),
      _desc(std::move(desc))
{
    _streamState->readStream = _readStream;
    _streamState->writeStream = _writeStream;
    _streamState->nativeInfo = _nativeInfo;
    _callback = [[iAPTransceiverCallback alloc] initWithState:_streamState];
}

IceObjC::iAPTransceiver::~iAPTransceiver()
{
#    if defined(__clang__) && !__has_feature(objc_arc)
    [_callback release];
    [_session release];
#    endif
    dispatch_release(_queue);
}

#endif
