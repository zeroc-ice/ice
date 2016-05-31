// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
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
#include <IceSSL/ConnectionInfo.h>

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

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
    CFWriteStreamSetClient(_writeStream, events, selectorWriteCallback, &ctx);

    events = kCFStreamEventOpenCompleted | kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred |
        kCFStreamEventEndEncountered;
    CFReadStreamSetClient(_readStream, events, selectorReadCallback, &ctx);
}

SocketOperation
IceObjC::StreamTransceiver::registerWithRunLoop(SocketOperation op)
{
    IceUtil::Mutex::Lock sync(_mutex);
    SocketOperation readyOp = SocketOperationNone;
    if(op & SocketOperationConnect)
    {
        if(CFWriteStreamGetStatus(_writeStream) != kCFStreamStatusNotOpen ||
           CFReadStreamGetStatus(_readStream) != kCFStreamStatusNotOpen)
        {
            return SocketOperationConnect;
        }

        _opening = true;

        CFWriteStreamScheduleWithRunLoop(_writeStream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        CFReadStreamScheduleWithRunLoop(_readStream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

        _writeStreamRegistered = true; // Note: this must be set after the schedule call
        _readStreamRegistered = true; // Note: this must be set after the schedule call

        CFReadStreamOpen(_readStream);
        CFWriteStreamOpen(_writeStream);
    }
    else
    {
        if(op & SocketOperationWrite)
        {
            if(CFWriteStreamCanAcceptBytes(_writeStream))
            {
                readyOp = static_cast<SocketOperation>(readyOp | SocketOperationWrite);
            }
            else if(!_writeStreamRegistered)
            {
                CFWriteStreamScheduleWithRunLoop(_writeStream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
                _writeStreamRegistered = true; // Note: this must be set after the schedule call
                if(CFWriteStreamCanAcceptBytes(_writeStream))
                {
                    readyOp = static_cast<SocketOperation>(readyOp | SocketOperationWrite);
                }
            }
        }

        if(op & SocketOperationRead)
        {
            if(CFReadStreamHasBytesAvailable(_readStream))
            {
                readyOp = static_cast<SocketOperation>(readyOp | SocketOperationRead);
            }
            else if(!_readStreamRegistered)
            {
                CFReadStreamScheduleWithRunLoop(_readStream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
                _readStreamRegistered = true; // Note: this must be set after the schedule call
                if(CFReadStreamHasBytesAvailable(_readStream))
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
            CFWriteStreamUnscheduleFromRunLoop(_writeStream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
            CFReadStreamUnscheduleFromRunLoop(_readStream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
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
            CFWriteStreamUnscheduleFromRunLoop(_writeStream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
            _writeStreamRegistered = false;
        }

        if(op & SocketOperationRead && _readStreamRegistered)
        {
            CFReadStreamUnscheduleFromRunLoop(_readStream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
            _readStreamRegistered = false;
        }
    }
    return op;
}

void
IceObjC::StreamTransceiver::closeStreams()
{
    CFReadStreamSetClient(_readStream, kCFStreamEventNone, 0, 0);
    CFWriteStreamSetClient(_writeStream, kCFStreamEventNone, 0, 0);

    CFReadStreamClose(_readStream);
    CFWriteStreamClose(_writeStream);
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
            CFErrorRef err = NULL;
            if(CFWriteStreamGetStatus(_writeStream) == kCFStreamStatusError)
            {
                err = CFWriteStreamCopyError(_writeStream);
            }
            else if(CFReadStreamGetStatus(_readStream) == kCFStreamStatusError)
            {
                err = CFReadStreamCopyError(_readStream);
            }
            checkError(err, __FILE__, __LINE__);
        }

        _state = StateConnected;

        if(_fd == INVALID_SOCKET)
        {
            if(!CFReadStreamSetProperty(_readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanFalse) ||
               !CFWriteStreamSetProperty(_writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanFalse))
            {
                throw Ice::SocketException(__FILE__, __LINE__, 0);
            }

            CFDataRef d = (CFDataRef)CFReadStreamCopyProperty(_readStream, kCFStreamPropertySocketNativeHandle);
            CFDataGetBytes(d, CFRangeMake(0, sizeof(SOCKET)), reinterpret_cast<UInt8*>(&_fd));
            CFRelease(d);
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

        //
        // Limit the size of packets passed to SSLWrite/SSLRead to avoid
        // blocking and holding too much memory.
        //
        _maxSendPacketSize = std::max(512, getSendBufferSize(_fd));
        _maxRecvPacketSize = std::max(512, getRecvBufferSize(_fd));
    }
    assert(_state == StateConnected);
    return SocketOperationNone;
}

SocketOperation
#ifdef ICE_CPP11_MAPPING
IceObjC::StreamTransceiver::closing(bool initiator, exception_ptr)
#else
IceObjC::StreamTransceiver::closing(bool initiator, const Ice::LocalException&)
#endif
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
        assert(CFWriteStreamGetStatus(_writeStream) == kCFStreamStatusError);
        checkError(CFWriteStreamCopyError(_writeStream), __FILE__, __LINE__);
    }

    // Its impossible for the packetSize to be more than an Int.
    size_t packetSize = std::min(static_cast<size_t>(buf.b.end() - buf.i), _maxSendPacketSize);
    while(buf.i != buf.b.end())
    {
        if(!CFWriteStreamCanAcceptBytes(_writeStream))
        {
            return SocketOperationWrite;
        }

        if(_checkCertificates)
        {
            _checkCertificates = false;
            checkCertificates();
        }

        assert(_fd != INVALID_SOCKET);
        CFIndex ret = CFWriteStreamWrite(_writeStream, reinterpret_cast<const UInt8*>(&*buf.i), packetSize);

        if(ret == SOCKET_ERROR)
        {
            if(CFWriteStreamGetStatus(_writeStream) == kCFStreamStatusAtEnd)
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }

            assert(CFWriteStreamGetStatus(_writeStream) == kCFStreamStatusError);
            checkError(CFWriteStreamCopyError(_writeStream), __FILE__, __LINE__);
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
IceObjC::StreamTransceiver::read(Buffer& buf)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_error)
    {
        assert(CFReadStreamGetStatus(_readStream) == kCFStreamStatusError);
        checkError(CFReadStreamCopyError(_readStream), __FILE__, __LINE__);
    }

    // Its impossible for the packetSize to be more than an Int.
    size_t packetSize = std::min(static_cast<size_t>(buf.b.end() - buf.i), _maxRecvPacketSize);
    while(buf.i != buf.b.end())
    {
        if(!CFReadStreamHasBytesAvailable(_readStream))
        {
            return SocketOperationRead;
        }

        if(_checkCertificates)
        {
            _checkCertificates = false;
            checkCertificates();
        }

        assert(_fd != INVALID_SOCKET);
        CFIndex ret = CFReadStreamRead(_readStream, reinterpret_cast<UInt8*>(&*buf.i), packetSize);

        if(ret == 0)
        {
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }

        if(ret == SOCKET_ERROR)
        {
            if(CFReadStreamGetStatus(_readStream) == kCFStreamStatusAtEnd)
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }

            assert(CFReadStreamGetStatus(_readStream) == kCFStreamStatusError);
            checkError(CFReadStreamCopyError(_readStream), __FILE__, __LINE__);
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
    if(_instance->secure())
    {
        IceSSL::ConnectionInfoPtr info = ICE_MAKE_SHARED(IceSSL::ConnectionInfo);
        fillConnectionInfo(info);
        info->verified = _state == StateConnected;
        return info;
    }
    else
    {
        Ice::TCPConnectionInfoPtr info = ICE_MAKE_SHARED(Ice::TCPConnectionInfo);
        fillConnectionInfo(info);
        return info;
    }
}

Ice::ConnectionInfoPtr
IceObjC::StreamTransceiver::getWSInfo(const Ice::HeaderDict& headers) const
{
    if(_instance->secure())
    {
        IceSSL::WSSConnectionInfoPtr info = ICE_MAKE_SHARED(IceSSL::WSSConnectionInfo);
        fillConnectionInfo(info);
        info->verified = _state == StateConnected;
        info->headers = headers;
        return info;
    }
    else
    {
        Ice::WSConnectionInfoPtr info = ICE_MAKE_SHARED(Ice::WSConnectionInfo);
        fillConnectionInfo(info);
        info->headers = headers;
        return info;
    }
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
    _checkCertificates(instance->secure()),
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
    _checkCertificates(false),
    _error(false),
    _state(StateNeedConnect),
    _desc(fdToString(fd))
{
}

IceObjC::StreamTransceiver::~StreamTransceiver()
{
    assert(_fd == INVALID_SOCKET);
    CFRelease(_readStream);
    CFRelease(_writeStream);
}

void
IceObjC::StreamTransceiver::checkCertificates()
{
    SecTrustRef trust = (SecTrustRef)CFWriteStreamCopyProperty(_writeStream, kCFStreamPropertySSLPeerTrust);
    if(!trust)
    {
        throw Ice::SecurityException(__FILE__, __LINE__, "unable to obtain trust object");
    }

    try
    {
        SecPolicyRef policy = 0;
        if(_host.empty() || _instance->properties()->getPropertyAsIntWithDefault("IceSSL.CheckCertName", 1) == 0)
        {
            policy = SecPolicyCreateBasicX509();
        }
        else
        {
            CFStringRef h = CFStringCreateWithCString(NULL, _host.c_str(), kCFStringEncodingUTF8);
            policy = SecPolicyCreateSSL(false, h);
            CFRelease(h);
        }

        OSStatus err = SecTrustSetPolicies(trust, policy);
        CFRelease(policy);
        if(err != noErr)
        {
            ostringstream os;
            os << "unable to set trust object policy (error = " << err << ")";
            throw Ice::SecurityException(__FILE__, __LINE__, os.str());
        }

        //
        // If IceSSL.CertAuthFile is set, we use the certificate authorities from this file
        // instead of the ones from the keychain.
        //
        if((err = SecTrustSetAnchorCertificates(trust, _instance->certificateAuthorities())) != noErr)
        {
            ostringstream os;
            os << "couldn't set root CA certificates with trust object (error = " << err << ")";
            throw Ice::SecurityException(__FILE__, __LINE__, os.str());
        }

        SecTrustResultType result = kSecTrustResultInvalid;
        if((err = SecTrustEvaluate(trust, &result)) != noErr)
        {
            ostringstream os;
            os << "unable to evaluate the peer certificate trust (error = " << err << ")";
            throw Ice::SecurityException(__FILE__, __LINE__, os.str());
        }

        //
        // The kSecTrustResultUnspecified result indicates that the user didn't set any trust
        // settings for the root CA. This is expected if the root CA is provided by the user
        // with IceSSL.CertAuthFile or if the user didn't explicitly set any trust settings
        // for the certificate.
        //
        if(result != kSecTrustResultProceed && result != kSecTrustResultUnspecified)
        {
            ostringstream os;
            os << "certificate validation failed (result = " << result << ")";
            throw Ice::SecurityException(__FILE__, __LINE__, os.str());
        }

        if(_instance->trustOnlyKeyID())
        {
            if(SecTrustGetCertificateCount(trust) < 0)
            {
                throw Ice::SecurityException(__FILE__, __LINE__, "unable to obtain peer certificate");
            }

            SecCertificateRef cert = SecTrustGetCertificateAtIndex(trust, 0);

            //
            // To check the subject key ID, we add the peer certificate to the keychain with SetItemAdd,
            // then we lookup for the cert using the kSecAttrSubjectKeyID. Then we remove the cert from
            // the keychain. NOTE: according to the Apple documentation, it should in theory be possible
            // to not add/remove the item to the keychain by specifying the kSecMatchItemList key (or
            // kSecUseItemList?) when calling SecItemCopyMatching. Unfortunately this doesn't appear to
            // work. Similarly, it should be possible to get back the attributes of the certificate
            // once it added by setting kSecReturnAttributes in the add query, again this doesn't seem
            // to work.
            //
            CFMutableDictionaryRef query;
            query = CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            CFDictionarySetValue(query, kSecClass, kSecClassCertificate);
            CFDictionarySetValue(query, kSecValueRef, cert);
            err = SecItemAdd(query, 0);
            if(err != noErr && err != errSecDuplicateItem)
            {
                CFRelease(query);
                ostringstream os;
                os << "unable to add peer certificate to keychain (error = " << err << ")";
                throw Ice::SecurityException(__FILE__, __LINE__, os.str());
            }
            CFRelease(query);

            query = CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            CFDictionarySetValue(query, kSecClass, kSecClassCertificate);
            CFDictionarySetValue(query, kSecValueRef, cert);
            CFDictionarySetValue(query, kSecAttrSubjectKeyID, _instance->trustOnlyKeyID());
            err = SecItemCopyMatching(query, 0);
            OSStatus foundErr = err;
            CFRelease(query);

            query = CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            CFDictionarySetValue(query, kSecClass, kSecClassCertificate);
            CFDictionarySetValue(query, kSecValueRef, cert);
            err = SecItemDelete(query);
            if(err != noErr)
            {
                CFRelease(query);
                ostringstream os;
                os << "unable to remove peer certificate from keychain (error = " << err << ")";
                throw Ice::SecurityException(__FILE__, __LINE__, os.str());
            }
            CFRelease(query);

            if(foundErr != noErr)
            {
                ostringstream os;
                os << "the certificate subject key ID doesn't match the `IceSSL.TrustOnly.Client' property ";
                os << "(error = " << foundErr << ")";
                throw Ice::SecurityException(__FILE__, __LINE__, os.str());
            }
        }
        CFRelease(trust);
    }
    catch(...)
    {
        if(trust)
        {
            CFRelease(trust);
        }
        throw;
    }
}

void
IceObjC::StreamTransceiver::checkError(CFErrorRef err, const char* file, int line)
{
    assert(err);
    CFStringRef domain = CFErrorGetDomain(err);
    if(CFStringCompare(domain, kCFErrorDomainPOSIX, 0) == kCFCompareEqualTo)
    {
        errno = CFErrorGetCode(err);
        CFRelease(err);
        if(interrupted() || noBuffers())
        {
            return;
        }

        if(connectionLost())
        {
            ConnectionLostException ex(file, line);
            ex.error = getSocketErrno();
            throw ex;
        }
        else if(connectionRefused())
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

    int error = CFErrorGetCode(err);
    if(error == kCFHostErrorHostNotFound || error == kCFHostErrorUnknown)
    {
        int rs = 0;
        if(error == kCFHostErrorUnknown)
        {
            CFDictionaryRef dict = CFErrorCopyUserInfo(err);
            CFNumberRef d = (CFNumberRef)CFDictionaryGetValue(dict, kCFGetAddrInfoFailureKey);
            if(d != 0)
            {
                CFNumberGetValue(d, kCFNumberSInt32Type, &rs);
            }
            CFRelease(dict);
        }

        CFRelease(err);

        DNSException ex(file, line);
        ex.error = rs;
        ex.host = _host;
        throw ex;
    }

    CFNetworkException ex(file, line);
    ex.domain = fromCFString(domain);
    ex.error = CFErrorGetCode(err);
    CFRelease(err);
    throw ex;
}

void
IceObjC::StreamTransceiver::fillConnectionInfo(const Ice::IPConnectionInfoPtr& info) const
{
    fdToAddressAndPort(_fd, info->localAddress, info->localPort, info->remoteAddress, info->remotePort);
    info->rcvSize = getRecvBufferSize(_fd);
    info->sndSize = getSendBufferSize(_fd);
}
