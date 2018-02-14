// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/winrt/StreamTransceiver.h>
#include <Ice/Connection.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Buffer.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>

#include <IceSSL/EndpointInfo.h>
#include <IceSSL/ConnectionInfo.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;

namespace
{

AsyncOperationCompletedHandler<unsigned int>^
createAsyncOperationCompletedHandler(SocketOperationCompletedHandler^ cb, SocketOperation op, AsyncInfo& info)
{
    return ref new AsyncOperationCompletedHandler<unsigned int>(
        [=,&info] (IAsyncOperation<unsigned int>^ operation, Windows::Foundation::AsyncStatus status)
        {
            if(status != Windows::Foundation::AsyncStatus::Completed)
            {
                info.count = SOCKET_ERROR;
                info.error = operation->ErrorCode.Value;
            }
            else
            {
                info.count = static_cast<int>(operation->GetResults());
            }
            cb(op);
        });
}

}

NativeInfoPtr
IceInternal::StreamTransceiver::getNativeInfo()
{
    return this;
}

void
IceInternal::StreamTransceiver::setCompletedHandler(SocketOperationCompletedHandler^ handler)
{
    _completedHandler = handler;
    _readOperationCompletedHandler = createAsyncOperationCompletedHandler(handler, SocketOperationRead, _read);
    _writeOperationCompletedHandler = createAsyncOperationCompletedHandler(handler, SocketOperationWrite, _write);
}

SocketOperation
IceInternal::StreamTransceiver::initialize(Buffer&, Buffer&,bool&)
{
    if(_state == StateNeedConnect)
    {
        _state = StateConnectPending;
        return SocketOperationConnect;
    }
    else if(_state <= StateConnectPending)
    {
         if(_write.count == SOCKET_ERROR)
        {
            checkConnectErrorCode(__FILE__, __LINE__, _write.error, _connectAddr.host);
        }
        _state = StateConnected;
        _desc = fdToString(_fd);
    }
    assert(_state == StateConnected);
    return SocketOperationNone;
}

SocketOperation
IceInternal::StreamTransceiver::closing(bool initiator, const Ice::LocalException&)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? SocketOperationRead : SocketOperationNone;
}

void
IceInternal::StreamTransceiver::close()
{
    assert(_fd != INVALID_SOCKET);

    _completedHandler = nullptr;
    _readOperationCompletedHandler = nullptr;
    _writeOperationCompletedHandler = nullptr;

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

SocketOperation
IceInternal::StreamTransceiver::write(Buffer& buf)
{
    return buf.i == buf.b.end() ? SocketOperationNone : SocketOperationWrite;
}

SocketOperation
IceInternal::StreamTransceiver::read(Buffer& buf, bool&)
{
    return buf.i == buf.b.end() ? SocketOperationNone : SocketOperationRead;
}

bool
IceInternal::StreamTransceiver::startWrite(Buffer& buf)
{
    if(_state < StateConnected)
    {
        try
        {
            IAsyncAction^ action = safe_cast<StreamSocket^>(_fd)->ConnectAsync(
                _connectAddr.host,
                _connectAddr.port,
                _instance->secure() ?
                    //
                    // SocketProtectionLevel::Tls12 is new in Windows 8.1 SDK
                    //
#if defined(_MSC_VER) && _MSC_VER >= 1800
                    SocketProtectionLevel::Tls12 :
#else
                    SocketProtectionLevel::Ssl :
#endif
                    SocketProtectionLevel::PlainSocket);

            if(!checkIfErrorOrCompleted(SocketOperationConnect, action))
            {
                SocketOperationCompletedHandler^ completed = _completedHandler;
                action->Completed = ref new AsyncActionCompletedHandler(
                    [=] (IAsyncAction^ info, Windows::Foundation::AsyncStatus status)
                    {
                        if(status != Windows::Foundation::AsyncStatus::Completed)
                        {
                            _write.count = SOCKET_ERROR;
                            _write.error = info->ErrorCode.Value;
                        }
                        else
                        {
                            _write.count = 0;
                        }
                        completed(SocketOperationConnect);
                    });
            }
        }
        catch(Platform::Exception^ ex)
        {
            checkConnectErrorCode(__FILE__, __LINE__, ex->HResult, _connectAddr.host);
        }
        return false;
    }

    assert(!buf.b.empty());
    assert(buf.i != buf.b.end());

    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
    {
        packetSize = _maxSendPacketSize;
    }
    assert(packetSize > 0);
    _writer->WriteBytes(ref new Array<unsigned char>(&*buf.i, packetSize));
    try
    {
        DataWriterStoreOperation^ operation = _writer->StoreAsync();
        if(checkIfErrorOrCompleted(SocketOperationWrite, operation))
        {
            _write.count = operation->GetResults();
        }
        else
        {
            operation->Completed = _writeOperationCompletedHandler;
        }
    }
    catch(Platform::Exception^ ex)
    {
        checkErrorCode(__FILE__, __LINE__, ex->HResult);
    }
    return packetSize == static_cast<int>(buf.b.end() - buf.i);
}

void
IceInternal::StreamTransceiver::finishWrite(Buffer& buf)
{
    if(_state < StateConnected)
    {
        if(_write.count == SOCKET_ERROR)
        {
            checkConnectErrorCode(__FILE__, __LINE__, _write.error, _connectAddr.host);
        }
        _verified = true;
        return;
    }

    if(_write.count == SOCKET_ERROR)
    {
        checkErrorCode(__FILE__, __LINE__, _write.error);
    }

    buf.i += _write.count;
}

void
IceInternal::StreamTransceiver::startRead(Buffer& buf)
{
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
    {
        packetSize = _maxReceivePacketSize;
    }
    assert(!buf.b.empty() && buf.i != buf.b.end());

    try
    {
        DataReaderLoadOperation^ operation = _reader->LoadAsync(packetSize);
        if(checkIfErrorOrCompleted(SocketOperationRead, operation))
        {
            _read.count = operation->GetResults();
        }
        else
        {
            operation->Completed = _readOperationCompletedHandler;
        }
    }
    catch(Platform::Exception^ ex)
    {
        checkErrorCode(__FILE__, __LINE__, ex->HResult);
    }
}

void
IceInternal::StreamTransceiver::finishRead(Buffer& buf, bool& hasMoreData)
{
    if(_read.count == SOCKET_ERROR)
    {
        checkErrorCode(__FILE__, __LINE__, _read.error);
    }
    else if(_read.count == 0)
    {
        ConnectionLostException ex(__FILE__, __LINE__);
        ex.error = 0;
        throw ex;
    }

    try
    {
        auto data = ref new Platform::Array<unsigned char>(_read.count);
        _reader->ReadBytes(data);
        memcpy(&*buf.i, data->Data, _read.count);
    }
    catch(Platform::Exception^ ex)
    {
        checkErrorCode(__FILE__, __LINE__, ex->HResult);
    }

    buf.i += _read.count;
}

string
IceInternal::StreamTransceiver::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::StreamTransceiver::toString() const
{
    return _desc;
}

string
IceInternal::StreamTransceiver::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceInternal::StreamTransceiver::getInfo() const
{
    Ice::IPConnectionInfoPtr info;
    if(_instance->secure())
    {
        IceSSL::ConnectionInfoPtr sslInfo = new IceSSL::ConnectionInfo();
        sslInfo->verified = _verified;
        info = sslInfo;
    }
    else
    {
        info = new Ice::TCPConnectionInfo();
    }
    fillConnectionInfo(info);
    return info;
}

Ice::ConnectionInfoPtr
IceInternal::StreamTransceiver::getWSInfo(const Ice::HeaderDict& headers) const
{
    if(_instance->secure())
    {
        IceSSL::WSSConnectionInfoPtr info = new IceSSL::WSSConnectionInfo();
        info->verified = _verified;
        fillConnectionInfo(info);
        info->headers = headers;
        return info;
    }
    else
    {
        Ice::WSConnectionInfoPtr info = new Ice::WSConnectionInfo();
        fillConnectionInfo(info);
        info->headers = headers;
        return info;
    }
}

void
IceInternal::StreamTransceiver::checkSendSize(const Buffer&)
{
}

 void
 IceInternal::StreamTransceiver::setBufferSize(int rcvSize, int sndSize)
 {
    setTcpBufSize(_fd, rcvSize, sndSize, _instance);
 }

IceInternal::StreamTransceiver::StreamTransceiver(const ProtocolInstancePtr& instance, SOCKET fd, bool connected) :
    NativeInfo(fd),
    _instance(instance),
    _state(connected ? StateConnected : StateNeedConnect),
    _verified(false)
{
    StreamSocket^ streamSocket = safe_cast<StreamSocket^>(_fd);
    _writer = ref new DataWriter(streamSocket->OutputStream);
    _reader = ref new DataReader(streamSocket->InputStream);
    _reader->InputStreamOptions = InputStreamOptions::Partial;

    if(connected)
    {
        try
        {
            _desc = fdToString(_fd);
        }
        catch(const Ice::Exception&)
        {
            closeSocketNoThrow(_fd);
            throw;
        }
    }

    setTcpBufSize(_fd, _instance);

    _maxSendPacketSize = streamSocket->Control->OutboundBufferSizeInBytes / 2;
    if(_maxSendPacketSize < 512)
    {
        _maxSendPacketSize = 0;
    }

    _maxReceivePacketSize = instance->properties()->getPropertyAsIntWithDefault("Ice.TCP.RcvSize", 128 * 1024);
}

IceInternal::StreamTransceiver::~StreamTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}

void
IceInternal::StreamTransceiver::connect(const Address& addr)
{
    _connectAddr = addr;
}

bool
IceInternal::StreamTransceiver::checkIfErrorOrCompleted(SocketOperation op, IAsyncInfo^ info, int count)
{
    //
    // NOTE: It's important to only check for info->Status once as it
    // might change during the checks below (the Status can be changed
    // by the Windows thread pool concurrently).
    //
    // We consider that a canceled async status is the same as an
    // error. A canceled async status can occur if there's a timeout
    // and the socket is closed.
    //
    Windows::Foundation::AsyncStatus status = info->Status;
    if(status == Windows::Foundation::AsyncStatus::Completed)
    {
        _completedHandler(op);
        return true;
    }
    else if (status == Windows::Foundation::AsyncStatus::Started)
    {
        return false;
    }
    else
    {
        if(_state < StateConnected)
        {
            checkConnectErrorCode(__FILE__, __LINE__, info->ErrorCode.Value, _connectAddr.host);
        }
        else
        {
            checkErrorCode(__FILE__, __LINE__, info->ErrorCode.Value);
        }
        return true; // Prevent compiler warning.
    }
}

void
IceInternal::StreamTransceiver::fillConnectionInfo(const Ice::IPConnectionInfoPtr& info) const
{
    fdToAddressAndPort(_fd, info->localAddress, info->localPort, info->remoteAddress, info->remotePort);
    info->rcvSize = getRecvBufferSize(_fd);
    info->sndSize = getSendBufferSize(_fd);
}

