// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "StreamTransceiver.h"
#include "StreamEndpointI.h"

#include <Ice/Properties.h>
#include <Ice/TraceLevels.h>
#include <Ice/Connection.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>

#include <CoreFoundation/CoreFoundation.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

void selectorReadCallback(CFReadStreamRef, CFStreamEventType event, void* info)
{
    SelectorReadyCallback* callback = reinterpret_cast<SelectorReadyCallback*>(info);
    switch(event)
    {
    case kCFStreamEventOpenCompleted:
        callback->readyCallback(static_cast<SocketOperation>(SocketOperationConnect | SocketOperationRead));
        break;
    case kCFStreamEventHasBytesAvailable:
        callback->readyCallback(SocketOperationRead);
        break;
    default:
        callback->readyCallback(SocketOperationRead, -1); // Error
        break;
    }
}

void selectorWriteCallback(CFWriteStreamRef, CFStreamEventType event, void* info)
{
    SelectorReadyCallback* callback = reinterpret_cast<SelectorReadyCallback*>(info);
    switch(event)
    {
    case kCFStreamEventOpenCompleted:
        callback->readyCallback(static_cast<SocketOperation>(SocketOperationConnect | SocketOperationWrite));
        break;
    case kCFStreamEventCanAcceptBytes:
        callback->readyCallback(SocketOperationWrite);
        break;
    default:
        callback->readyCallback(SocketOperationWrite, -1); // Error
        break;
    }
}

}

static inline string
fromCFString(CFStringRef ref)
{
   const char* s = CFStringGetCStringPtr(ref, kCFStringEncodingUTF8);
   if(s)
   {
       return string(s);
   }

   // Not great, but is good enough for this purpose.
   char buf[1024];
   CFStringGetCString(ref, buf, sizeof(buf), kCFStringEncodingUTF8);
   return string(buf);
}

IceInternal::NativeInfoPtr
IceObjC::StreamTransceiver::getNativeInfo()
{
    return this;
}

void
IceObjC::StreamTransceiver::initStreams(SelectorReadyCallback* callback)
{
    CFOptionFlags events;
    CFStreamClientContext ctx = { 0, callback, 0, 0, 0 };
    events = kCFStreamEventOpenCompleted | kCFStreamEventCanAcceptBytes | kCFStreamEventErrorOccurred |
        kCFStreamEventEndEncountered;
    CFWriteStreamSetClient(_writeStream.get(), events, selectorWriteCallback, &ctx);

    events = kCFStreamEventOpenCompleted | kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred |
        kCFStreamEventEndEncountered;
    CFReadStreamSetClient(_readStream.get(), events, selectorReadCallback, &ctx);
}

SocketOperation
IceObjC::StreamTransceiver::registerWithRunLoop(SocketOperation op)
{
    IceUtil::Mutex::Lock sync(_mutex);
    SocketOperation readyOp = SocketOperationNone;
    if(op & SocketOperationConnect)
    {
        if(CFWriteStreamGetStatus(_writeStream.get()) != kCFStreamStatusNotOpen ||
           CFReadStreamGetStatus(_readStream.get()) != kCFStreamStatusNotOpen)
        {
            return SocketOperationConnect;
        }

        _opening = true;

        CFWriteStreamScheduleWithRunLoop(_writeStream.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        CFReadStreamScheduleWithRunLoop(_readStream.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

        _writeStreamRegistered = true; // Note: this must be set after the schedule call
        _readStreamRegistered = true; // Note: this must be set after the schedule call

        CFReadStreamOpen(_readStream.get());
        CFWriteStreamOpen(_writeStream.get());
    }
    else
    {
        if(op & SocketOperationWrite)
        {
            if(CFWriteStreamCanAcceptBytes(_writeStream.get()))
            {
                readyOp = static_cast<SocketOperation>(readyOp | SocketOperationWrite);
            }
            else if(!_writeStreamRegistered)
            {
                CFWriteStreamScheduleWithRunLoop(_writeStream.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
                _writeStreamRegistered = true; // Note: this must be set after the schedule call
                if(CFWriteStreamCanAcceptBytes(_writeStream.get()))
                {
                    readyOp = static_cast<SocketOperation>(readyOp | SocketOperationWrite);
                }
            }
        }

        if(op & SocketOperationRead)
        {
            if(CFReadStreamHasBytesAvailable(_readStream.get()))
            {
                readyOp = static_cast<SocketOperation>(readyOp | SocketOperationRead);
            }
            else if(!_readStreamRegistered)
            {
                CFReadStreamScheduleWithRunLoop(_readStream.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
                _readStreamRegistered = true; // Note: this must be set after the schedule call
                if(CFReadStreamHasBytesAvailable(_readStream.get()))
                {
                    readyOp = static_cast<SocketOperation>(readyOp | SocketOperationRead);
                }
            }
        }
    }
    return readyOp;
}

SocketOperation
IceObjC::StreamTransceiver::unregisterFromRunLoop(SocketOperation op, bool error)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _error |= error;

    if(_opening)
    {
        // Wait for the stream to be ready for write
        if(op & SocketOperationWrite)
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
        if(op & (SocketOperationRead | SocketOperationConnect))
        {
            _readStreamRegistered = false;
        }

        if(error || (!_readStreamRegistered && !_writeStreamRegistered))
        {
            CFWriteStreamUnscheduleFromRunLoop(_writeStream.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
            CFReadStreamUnscheduleFromRunLoop(_readStream.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
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
            CFWriteStreamUnscheduleFromRunLoop(_writeStream.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
            _writeStreamRegistered = false;
        }

        if(op & SocketOperationRead && _readStreamRegistered)
        {
            CFReadStreamUnscheduleFromRunLoop(_readStream.get(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
            _readStreamRegistered = false;
        }
    }
    return op;
}

void
IceObjC::StreamTransceiver::closeStreams()
{
    CFReadStreamSetClient(_readStream.get(), kCFStreamEventNone, 0, 0);
    CFWriteStreamSetClient(_writeStream.get(), kCFStreamEventNone, 0, 0);

    CFReadStreamClose(_readStream.get());
    CFWriteStreamClose(_writeStream.get());
}

SocketOperation
IceObjC::StreamTransceiver::initialize(Buffer& readBuffer, Buffer& writeBuffer)
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
            checkErrorStatus(_writeStream.get(), _readStream.get(), __FILE__, __LINE__);
        }

        _state = StateConnected;

        if(_fd == INVALID_SOCKET)
        {
            if(!CFReadStreamSetProperty(_readStream.get(), kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanFalse) ||
               !CFWriteStreamSetProperty(_writeStream.get(), kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanFalse))
            {
                throw Ice::SocketException(__FILE__, __LINE__, 0);
            }

            UniqueRef<CFDataRef> d(static_cast<CFDataRef>(
                CFReadStreamCopyProperty(_readStream.get(), kCFStreamPropertySocketNativeHandle)));
            CFDataGetBytes(d.get(), CFRangeMake(0, sizeof(SOCKET)), reinterpret_cast<UInt8*>(&_fd));
        }

        ostringstream s;
        Address localAddr;
        fdToLocalAddress(_fd, localAddr);
        s << "local address = " << addrToString(localAddr);
        string proxyHost = _instance->proxyHost();
        if(!proxyHost.empty())
        {
            s << "\nSOCKS proxy address = " << proxyHost << ":" << _instance->proxyPort();
        }
        Address remoteAddr;
        bool peerConnected = fdToRemoteAddress(_fd, remoteAddr);
        if(peerConnected)
        {
            s << "\nremote address = " << addrToString(remoteAddr);
        }
        else
        {
            s << "\nremote address = " << _host << ":" << _port;
        }
        _desc = s.str();

        setBlock(_fd, false);
        setTcpBufSize(_fd, _instance);
    }
    assert(_state == StateConnected);
    return SocketOperationNone;
}

SocketOperation
IceObjC::StreamTransceiver::closing(bool initiator, const Ice::LocalException&)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? SocketOperationRead : SocketOperationNone;
}

void
IceObjC::StreamTransceiver::close()
{
    if(_fd != INVALID_SOCKET)
    {
        try
        {
            closeSocket(_fd);
            _fd = INVALID_SOCKET;
        }
        catch(const SocketException&)
        {
            _fd = INVALID_SOCKET;
            throw;
        }
    }
}

SocketOperation
IceObjC::StreamTransceiver::write(Buffer& buf)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_error)
    {
        checkErrorStatus(_writeStream.get(), 0, __FILE__, __LINE__);
    }

    size_t packetSize = buf.b.end() - buf.i;
    while(buf.i != buf.b.end())
    {
        if(!CFWriteStreamCanAcceptBytes(_writeStream.get()))
        {
            return SocketOperationWrite;
        }

        assert(_fd != INVALID_SOCKET);
        CFIndex ret = CFWriteStreamWrite(_writeStream.get(), reinterpret_cast<const UInt8*>(&*buf.i), packetSize);

        if(ret == SOCKET_ERROR)
        {
            checkErrorStatus(_writeStream.get(), 0, __FILE__, __LINE__);
            if(noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
            }
            continue;
        }

        buf.i += ret;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = buf.b.end() - buf.i;
        }
    }
    return SocketOperationNone;
}

SocketOperation
IceObjC::StreamTransceiver::read(Buffer& buf)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_error)
    {
        checkErrorStatus(0, _readStream.get(), __FILE__, __LINE__);
    }

    size_t packetSize = buf.b.end() - buf.i;
    while(buf.i != buf.b.end())
    {
        if(!CFReadStreamHasBytesAvailable(_readStream.get()))
        {
            return SocketOperationRead;
        }

        assert(_fd != INVALID_SOCKET);
        CFIndex ret = CFReadStreamRead(_readStream.get(), reinterpret_cast<UInt8*>(&*buf.i), packetSize);

        if(ret == 0)
        {
            throw ConnectionLostException(__FILE__, __LINE__, 0);
        }

        if(ret == SOCKET_ERROR)
        {
            checkErrorStatus(0, _readStream.get(), __FILE__, __LINE__);
            if(noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
            }
            continue;
        }

        buf.i += ret;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = buf.b.end() - buf.i;
        }
    }

    return SocketOperationNone;
}

string
IceObjC::StreamTransceiver::protocol() const
{
    return _instance->protocol();
}

string
IceObjC::StreamTransceiver::toString() const
{
    return _desc;
}

string
IceObjC::StreamTransceiver::toDetailedString() const
{
    return _desc;
}

Ice::ConnectionInfoPtr
IceObjC::StreamTransceiver::getInfo() const
{
    Ice::TCPConnectionInfoPtr info = ICE_MAKE_SHARED(Ice::TCPConnectionInfo);
    fdToAddressAndPort(_fd, info->localAddress, info->localPort, info->remoteAddress, info->remotePort);
    info->rcvSize = getRecvBufferSize(_fd);
    info->sndSize = getSendBufferSize(_fd);
    return info;
}

void
IceObjC::StreamTransceiver::checkSendSize(const Buffer& buf)
{
}

void
IceObjC::StreamTransceiver::setBufferSize(int rcvSize, int sndSize)
{
    setTcpBufSize(_fd, rcvSize, sndSize, _instance);
}

IceObjC::StreamTransceiver::StreamTransceiver(const InstancePtr& instance,
                                              CFReadStreamRef readStream,
                                              CFWriteStreamRef writeStream,
                                              const string& host,
                                              Ice::Int port) :
    StreamNativeInfo(INVALID_SOCKET),
    _instance(instance),
    _host(host),
    _port(port),
    _readStream(readStream),
    _writeStream(writeStream),
    _readStreamRegistered(false),
    _writeStreamRegistered(false),
    _opening(false),
    _error(false),
    _state(StateNeedConnect)
{
    ostringstream s;
    s << "local address = <not available>";
    string proxyHost = instance->proxyHost();
    if(!proxyHost.empty())
    {
        s << "\nSOCKS proxy address = " << proxyHost << ":" << instance->proxyPort();
    }
    s << "\nremote address = " << host << ":" << port;
    _desc = s.str();
}

IceObjC::StreamTransceiver::StreamTransceiver(const InstancePtr& instance,
                                              CFReadStreamRef readStream,
                                              CFWriteStreamRef writeStream,
                                              SOCKET fd) :
    StreamNativeInfo(fd),
    _instance(instance),
    _port(0),
    _readStream(readStream),
    _writeStream(writeStream),
    _readStreamRegistered(false),
    _writeStreamRegistered(false),
    _opening(false),
    _error(false),
    _state(StateNeedConnect),
    _desc(fdToString(fd))
{
}

IceObjC::StreamTransceiver::~StreamTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}

void
IceObjC::StreamTransceiver::checkErrorStatus(CFWriteStreamRef writeStream, CFReadStreamRef readStream,
                                             const char* file, int line)
{
    if((writeStream && (CFWriteStreamGetStatus(writeStream) == kCFStreamStatusAtEnd)) ||
       (readStream && (CFReadStreamGetStatus(readStream) == kCFStreamStatusAtEnd)))
    {
        throw ConnectionLostException(file, line);
    }

    UniqueRef<CFErrorRef> err;
    if(writeStream && CFWriteStreamGetStatus(writeStream) == kCFStreamStatusError)
    {
        err.reset(CFWriteStreamCopyError(writeStream));
    }
    else if(readStream && CFReadStreamGetStatus(readStream) == kCFStreamStatusError)
    {
        err.reset(CFReadStreamCopyError(readStream));
    }

    assert(err.get());
    CFStringRef domain = CFErrorGetDomain(err.get());
    if(CFStringCompare(domain, kCFErrorDomainPOSIX, 0) == kCFCompareEqualTo)
    {
        errno = CFErrorGetCode(err.get());
        if(interrupted() || noBuffers())
        {
            return;
        }
        else if(connectionLost())
        {
            throw ConnectionLostException(file, line, getSocketErrno());
        }
        else if(connectionRefused())
        {
            throw ConnectionRefusedException(file, line, getSocketErrno());
        }
        else if(connectFailed())
        {
            throw ConnectFailedException(file, line, getSocketErrno());
        }
        else
        {
            throw SocketException(file, line, getSocketErrno());
        }
    }

    int error = CFErrorGetCode(err.get());
    if(error == kCFHostErrorHostNotFound || error == kCFHostErrorUnknown)
    {
        int rs = 0;
        if(error == kCFHostErrorUnknown)
        {
            UniqueRef<CFDictionaryRef> dict(CFErrorCopyUserInfo(err.get()));
            CFNumberRef d = static_cast<CFNumberRef>(CFDictionaryGetValue(dict.get(), kCFGetAddrInfoFailureKey));
            if(d != 0)
            {
                CFNumberGetValue(d, kCFNumberSInt32Type, &rs);
            }
        }
        throw DNSException(file, line, rs, _host);
    }
    throw CFNetworkException(file, line, CFErrorGetCode(err.get()), fromCFString(domain));
}
