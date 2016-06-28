// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "Transceiver.h"
#include "EndpointI.h"

#include <IceIAP/ConnectionInfo.h>

#include <Ice/LocalException.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/Buffer.h>

#import <Foundation/NSRunLoop.h>
#import <Foundation/NSError.h>
#import <Foundation/NSString.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

@interface iAPTransceiverCallback : NSObject<NSStreamDelegate>
{
@private

    SelectorReadyCallback* callback;
}
-(id) init:(SelectorReadyCallback*)cb;
@end

@implementation iAPTransceiverCallback
-(id) init:(SelectorReadyCallback*)cb;
{
    if(![super init])
    {
        return nil;
    }
    callback = cb;
    return self;
}

- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode
{
    switch(eventCode)
    {
    case NSStreamEventHasBytesAvailable:
        callback->readyCallback(SocketOperationRead);
        break;
    case NSStreamEventHasSpaceAvailable:
        callback->readyCallback(SocketOperationWrite);
        break;
    case NSStreamEventOpenCompleted:
        if([[stream class] isSubclassOfClass:[NSInputStream class]])
        {
            callback->readyCallback(static_cast<SocketOperation>(SocketOperationConnect | SocketOperationRead));
        }
        else
        {
            callback->readyCallback(static_cast<SocketOperation>(SocketOperationConnect | SocketOperationWrite));
        }
        break;
    default:
        if([[stream class] isSubclassOfClass:[NSInputStream class]])
        {
            callback->readyCallback(SocketOperationRead, -1); // Error
        }
        else
        {
            callback->readyCallback(SocketOperationWrite, -1); // Error
        }
    }
}
@end

void
IceObjC::iAPTransceiver::initStreams(SelectorReadyCallback* callback)
{
    _callback = [[iAPTransceiverCallback alloc] init:callback];
    [_writeStream setDelegate:_callback];
    [_readStream setDelegate:_callback];
}

SocketOperation
IceObjC::iAPTransceiver::registerWithRunLoop(SocketOperation op)
{
    IceUtil::Mutex::Lock sync(_mutex);
    SocketOperation readyOp = SocketOperationNone;
    if(op & SocketOperationConnect)
    {
        if([_writeStream streamStatus] != NSStreamStatusNotOpen || [_readStream streamStatus] != NSStreamStatusNotOpen)
        {
            return SocketOperationConnect;
        }

        _opening = true;

        [_writeStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [_readStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

        _writeStreamRegistered = true; // Note: this must be set after the schedule call
        _readStreamRegistered = true; // Note: this must be set after the schedule call

        [_writeStream open];
        [_readStream open];
    }
    else
    {
        if(op & SocketOperationWrite)
        {
            if([_writeStream hasSpaceAvailable])
            {
                readyOp = static_cast<SocketOperation>(readyOp | SocketOperationWrite);
            }
            else if(!_writeStreamRegistered)
            {
                [_writeStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
                _writeStreamRegistered = true; // Note: this must be set after the schedule call
                if([_writeStream hasSpaceAvailable])
                {
                    readyOp = static_cast<SocketOperation>(readyOp | SocketOperationWrite);
                }
            }
        }

        if(op & SocketOperationRead)
        {
            if([_readStream hasBytesAvailable])
            {
                readyOp = static_cast<SocketOperation>(readyOp | SocketOperationRead);
            }
            else if(!_readStreamRegistered)
            {
                [_readStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
                _readStreamRegistered = true; // Note: this must be set after the schedule call
                if([_readStream hasBytesAvailable])
                {
                    readyOp = static_cast<SocketOperation>(readyOp | SocketOperationRead);
                }
            }
        }
    }
    return readyOp;
}

SocketOperation
IceObjC::iAPTransceiver::unregisterFromRunLoop(SocketOperation op, bool error)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _error |= error;

    if(_opening)
    {
        // Wait for the stream to be ready for write
        if(op == SocketOperationWrite)
        {
            _writeStreamRegistered = false;
        }

        //
        // We don't wait for the stream to be ready for read (even if
        // it's a client connection) because there's no guarantees that
        // the server might actually send data right away. If we use
        // the WebSocket transport, the server actually waits for the
        // client to write the HTTP upgrade request.
        //
        //if(op & SocketOperationRead && (_fd != INVALID_SOCKET || !(op & SocketOperationConnect)))
        if(op == (SocketOperationRead | SocketOperationConnect))
        {
            _readStreamRegistered = false;
        }

        if(error || (!_readStreamRegistered && !_writeStreamRegistered))
        {
            [_writeStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            [_readStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            _opening = false;
            return SocketOperationConnect;
        }
        else
        {
            return SocketOperationNone;
        }
    }
    else
    {
        if(op & SocketOperationWrite && _writeStreamRegistered)
        {
            [_writeStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            _writeStreamRegistered = false;
        }

        if(op & SocketOperationRead && _readStreamRegistered)
        {
            [_readStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            _readStreamRegistered = false;
        }
    }
    return op;
}

void
IceObjC::iAPTransceiver::closeStreams()
{
    [_writeStream setDelegate:nil];
    [_readStream setDelegate:nil];

    [_callback release];
    _callback = 0;

    [_writeStream close];
    [_readStream close];
}

IceInternal::NativeInfoPtr
IceObjC::iAPTransceiver::getNativeInfo()
{
    return this;
}

SocketOperation
IceObjC::iAPTransceiver::initialize(Buffer& readBuffer, Buffer& writeBuffer)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_state == StateNeedConnect)
    {
        _state = StateConnectPending;
        return SocketOperationConnect;
    }

    if(_state <= StateConnectPending)
    {
        if(_error)
        {
            NSError* err = nil;
            if([_writeStream streamStatus] == NSStreamStatusError)
            {
                err = [_writeStream streamError];
            }
            if([_readStream streamStatus] == NSStreamStatusError)
            {
                err = [_readStream streamError];
            }
            checkError(err, __FILE__, __LINE__);
        }
        _state = StateConnected;
    }
    assert(_state == StateConnected);
    return SocketOperationNone;
}

SocketOperation
#ifdef ICE_CPP11_MAPPING
IceObjC::iAPTransceiver::closing(bool initiator, exception_ptr)
#else
IceObjC::iAPTransceiver::closing(bool initiator, const Ice::LocalException&)
#endif
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? SocketOperationRead : SocketOperationNone;
}

void
IceObjC::iAPTransceiver::close()
{
}

SocketOperation
IceObjC::iAPTransceiver::write(Buffer& buf)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_error)
    {
        assert([_writeStream streamStatus] == NSStreamStatusError);
        checkError([_writeStream streamError], __FILE__, __LINE__);
    }

    // Its impossible for the packetSize to be more than an Int.
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    while(buf.i != buf.b.end())
    {
        if(![_writeStream hasSpaceAvailable])
        {
            return SocketOperationWrite;
        }
        assert([_writeStream streamStatus] >= NSStreamStatusOpen);

        NSInteger ret = [_writeStream write:reinterpret_cast<const UInt8*>(&*buf.i) maxLength:packetSize];
        if(ret == SOCKET_ERROR)
        {
            if([_writeStream streamStatus] == NSStreamStatusAtEnd)
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }

            assert([_writeStream streamStatus] == NSStreamStatusError);
            checkError([_writeStream streamError], __FILE__, __LINE__);
            if(noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
            }
            continue;
        }

        buf.i += ret;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = static_cast<int>(buf.b.end() - buf.i);
        }
    }

    return SocketOperationNone;
}

SocketOperation
IceObjC::iAPTransceiver::read(Buffer& buf)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_error)
    {
        assert([_readStream streamStatus] == NSStreamStatusError);
        checkError([_readStream streamError], __FILE__, __LINE__);
    }

    // Its impossible for the packetSize to be more than an Int.
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    while(buf.i != buf.b.end())
    {
        if(![_readStream hasBytesAvailable] && [_readStream streamStatus] != NSStreamStatusError)
        {
            return SocketOperationRead;
        }
        assert([_readStream streamStatus] >= NSStreamStatusOpen);

        NSInteger ret = [_readStream read:reinterpret_cast<UInt8*>(&*buf.i) maxLength:packetSize];
        if(ret == 0)
        {
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }

        if(ret == SOCKET_ERROR)
        {
            if([_readStream streamStatus] == NSStreamStatusAtEnd)
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }

            assert([_readStream streamStatus] == NSStreamStatusError);
            checkError([_readStream streamError], __FILE__, __LINE__);
            if(noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
            }
            continue;
        }

        buf.i += ret;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = static_cast<int>(buf.b.end() - buf.i);
        }
    }

    return SocketOperationNone;
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
IceObjC::iAPTransceiver::getInfo() const
{
    IceIAP::ConnectionInfoPtr info = ICE_MAKE_SHARED(IceIAP::ConnectionInfo);
    info->manufacturer = [_session.accessory.manufacturer UTF8String];
    info->name = [_session.accessory.name UTF8String];
    info->modelNumber = [_session.accessory.modelNumber UTF8String];
    info->firmwareRevision = [_session.accessory.firmwareRevision UTF8String];
    info->hardwareRevision = [_session.accessory.hardwareRevision UTF8String];
    info->protocol = [_session.protocolString UTF8String];
    return info;
}

void
IceObjC::iAPTransceiver::checkSendSize(const Buffer& buf)
{
}

void
IceObjC::iAPTransceiver::setBufferSize(int, int)
{
}

IceObjC::iAPTransceiver::iAPTransceiver(const ProtocolInstancePtr& instance, EASession* session) :
    StreamNativeInfo(INVALID_SOCKET),
    _instance(instance),
    _session([session retain]),
    _readStream([session inputStream]),
    _writeStream([session outputStream]),
    _readStreamRegistered(false),
    _writeStreamRegistered(false),
    _error(false),
    _state(StateNeedConnect)
{
    ostringstream os;
    os << "name = " << [session.accessory.name UTF8String] << "\n";
    os << "protocol = " << [session.protocolString UTF8String];
    _desc = os.str();
}

IceObjC::iAPTransceiver::~iAPTransceiver()
{
    [_session release];
}

void
IceObjC::iAPTransceiver::checkError(NSError* err, const char* file, int line)
{
    NSString* domain = [err domain];
    if([domain compare:NSPOSIXErrorDomain] == NSOrderedSame)
    {
        errno = [err code];
        [err release];
        if(connectionRefused())
        {
            ConnectionRefusedException ex(file, line);
            ex.error = getSocketErrno();
            throw ex;
        }
        else if(connectFailed())
        {
            ConnectFailedException ex(file, line);
            ex.error = getSocketErrno();
            throw ex;
        }
        else
        {
            SocketException ex(file, line);
            ex.error = getSocketErrno();
            throw ex;
        }
    }

    // Otherwise throw a generic exception.
    CFNetworkException ex(file, line);
    ex.domain = [domain UTF8String];
    ex.error = [err code];
    [err release];
    throw ex;
}
