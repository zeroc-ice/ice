// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/uwp/TransceiverI.h>
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
using namespace IceSSL;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;

namespace
{

AsyncOperationCompletedHandler<unsigned int>^
createAsyncOperationCompletedHandler(IceInternal::SocketOperationCompletedHandler^ cb, IceInternal::SocketOperation op, IceInternal::AsyncInfo& info)
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

IceInternal::NativeInfoPtr
IceSSL::TransceiverI::getNativeInfo()
{
    return this;
}

void
IceSSL::TransceiverI::setCompletedHandler(IceInternal::SocketOperationCompletedHandler^ handler)
{
    _completedHandler = handler;
    _readOperationCompletedHandler = createAsyncOperationCompletedHandler(handler, IceInternal::SocketOperationRead, _read);
    _writeOperationCompletedHandler = createAsyncOperationCompletedHandler(handler, IceInternal::SocketOperationWrite, _write);
}

IceInternal::SocketOperation
IceSSL::TransceiverI::initialize(IceInternal::Buffer&, IceInternal::Buffer&)
{
    if(_state == StateNeedConnect)
    {
        _state = StateConnectPending;
        return IceInternal::SocketOperationConnect;
    }
    else if(_state <= StateConnectPending)
    {
         if(_write.count == SOCKET_ERROR)
        {
            IceInternal::checkConnectErrorCode(__FILE__, __LINE__, _write.error, _connectAddr.host);
        }
        _state = StateConnected;
        _desc = IceInternal::fdToString(_fd);
    }
    assert(_state == StateConnected);
    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
#ifdef ICE_CPP11_MAPPING
IceSSL::TransceiverI::closing(bool initiator, exception_ptr)
#else
IceSSL::TransceiverI::closing(bool initiator, const Ice::LocalException&)
#endif
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
IceSSL::TransceiverI::close()
{
    assert(_fd != INVALID_SOCKET);

    _completedHandler = nullptr;
    _readOperationCompletedHandler = nullptr;
    _writeOperationCompletedHandler = nullptr;

    try
    {
        IceInternal::closeSocket(_fd);
        _fd = INVALID_SOCKET;
    }
    catch(const SocketException&)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
}

IceInternal::SocketOperation
IceSSL::TransceiverI::write(IceInternal::Buffer& buf)
{
    return buf.i == buf.b.end() ? IceInternal::SocketOperationNone : IceInternal::SocketOperationWrite;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::read(IceInternal::Buffer& buf)
{
    return buf.i == buf.b.end() ? IceInternal::SocketOperationNone : IceInternal::SocketOperationRead;
}

bool
IceSSL::TransceiverI::startWrite(IceInternal::Buffer& buf)
{
    if(_state < StateConnected)
    {
        try
        {
            IAsyncAction^ action = safe_cast<StreamSocket^>(_fd)->ConnectAsync(_connectAddr.host,
                                                                               _connectAddr.port,
                                                                               SocketProtectionLevel::Tls12);

            if(!checkIfErrorOrCompleted(IceInternal::SocketOperationConnect, action))
            {
                IceInternal::SocketOperationCompletedHandler^ completed = _completedHandler;
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
                        completed(IceInternal::SocketOperationConnect);
                    });
            }
        }
        catch(Platform::Exception^ ex)
        {
            IceInternal::checkConnectErrorCode(__FILE__, __LINE__, ex->HResult, _connectAddr.host);
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
        if(checkIfErrorOrCompleted(IceInternal::SocketOperationWrite, operation))
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
        IceInternal::checkErrorCode(__FILE__, __LINE__, ex->HResult);
    }
    return packetSize == static_cast<int>(buf.b.end() - buf.i);
}

void
IceSSL::TransceiverI::finishWrite(IceInternal::Buffer& buf)
{
    if(_state < StateConnected)
    {
        if(_write.count == SOCKET_ERROR)
        {
            IceInternal::checkConnectErrorCode(__FILE__, __LINE__, _write.error, _connectAddr.host);
        }
        _verified = true;
        return;
    }

    if(_write.count == SOCKET_ERROR)
    {
        IceInternal::checkErrorCode(__FILE__, __LINE__, _write.error);
    }

    buf.i += _write.count;
}

void
IceSSL::TransceiverI::startRead(IceInternal::Buffer& buf)
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
        if(checkIfErrorOrCompleted(IceInternal::SocketOperationRead, operation))
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
        IceInternal::checkErrorCode(__FILE__, __LINE__, ex->HResult);
    }
}

void
IceSSL::TransceiverI::finishRead(IceInternal::Buffer& buf)
{
    if(_read.count == SOCKET_ERROR)
    {
        IceInternal::checkErrorCode(__FILE__, __LINE__, _read.error);
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
        IceInternal::checkErrorCode(__FILE__, __LINE__, ex->HResult);
    }

    buf.i += _read.count;
}

string
IceSSL::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
IceSSL::TransceiverI::toString() const
{
    return _desc;
}

string
IceSSL::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceSSL::TransceiverI::getInfo() const
{
    Ice::IPConnectionInfoPtr info;
    if(_instance->secure())
    {
        IceSSL::ConnectionInfoPtr sslInfo = ICE_MAKE_SHARED(IceSSL::ConnectionInfo);
        sslInfo->verified = _verified;
        info = sslInfo;
    }
    else
    {
        info = ICE_MAKE_SHARED(Ice::TCPConnectionInfo);
    }
    fillConnectionInfo(info);
    return info;
}

Ice::ConnectionInfoPtr
IceSSL::TransceiverI::getWSInfo(const Ice::HeaderDict& headers) const
{
    if(_instance->secure())
    {
        IceSSL::WSSConnectionInfoPtr info = ICE_MAKE_SHARED(IceSSL::WSSConnectionInfo);
        info->verified = _verified;
        fillConnectionInfo(info);
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
IceSSL::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

 void
 IceSSL::TransceiverI::setBufferSize(int rcvSize, int sndSize)
 {
    setTcpBufSize(_fd, rcvSize, sndSize, _instance);
 }

IceSSL::TransceiverI::TransceiverI(const IceInternal::ProtocolInstancePtr& instance, SOCKET fd, bool connected) :
    NativeInfo(fd),
    _instance(instance),
    _state(connected ? StateConnected : StateNeedConnect),
    _desc(connected ? IceInternal::fdToString(_fd) : string()),
    _verified(false)
{
    StreamSocket^ streamSocket = safe_cast<StreamSocket^>(_fd);
    _writer = ref new DataWriter(streamSocket->OutputStream);
    _reader = ref new DataReader(streamSocket->InputStream);
    _reader->InputStreamOptions = InputStreamOptions::Partial;

    setTcpBufSize(_fd, _instance);

    _maxSendPacketSize = streamSocket->Control->OutboundBufferSizeInBytes / 2;
    if(_maxSendPacketSize < 512)
    {
        _maxSendPacketSize = 0;
    }

    _maxReceivePacketSize = instance->properties()->getPropertyAsIntWithDefault("Ice.TCP.RcvSize", 128 * 1024);
}

IceSSL::TransceiverI::~TransceiverI()
{
    assert(_fd == INVALID_SOCKET);
}

void
IceSSL::TransceiverI::connect(const IceInternal::Address& addr)
{
    _connectAddr = addr;
}

bool
IceSSL::TransceiverI::checkIfErrorOrCompleted(IceInternal::SocketOperation op, IAsyncInfo^ info, int count)
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
            IceInternal::checkConnectErrorCode(__FILE__, __LINE__, info->ErrorCode.Value, _connectAddr.host);
        }
        else
        {
            IceInternal::checkErrorCode(__FILE__, __LINE__, info->ErrorCode.Value);
        }
        return true; // Prevent compiler warning.
    }
}

void
IceSSL::TransceiverI::fillConnectionInfo(const Ice::IPConnectionInfoPtr& info) const
{
    IceInternal::fdToAddressAndPort(_fd, info->localAddress, info->localPort, info->remoteAddress, info->remotePort);
    info->rcvSize = IceInternal::getRecvBufferSize(_fd);
    info->sndSize = IceInternal::getSendBufferSize(_fd);
}
