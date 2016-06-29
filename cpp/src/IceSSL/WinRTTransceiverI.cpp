// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/Config.h>

#ifdef ICE_OS_WINRT

#include <IceSSL/WinRTTransceiverI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

using namespace Platform;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;

IceInternal::NativeInfoPtr
IceSSL::TransceiverI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

IceInternal::SocketOperation
IceSSL::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
{
    if(!_connected)
    {
        IceInternal::SocketOperation status = _delegate->initialize(readBuffer, writeBuffer);
        if(status != IceInternal::SocketOperationNone)
        {
            return status;
        }
        _connected = true;

        //
        // Continue connecting, this will call startWrite/finishWrite to upgrade the stream
        // to SSL.
        //
        return IceInternal::SocketOperationConnect;
    }
    else if(!_upgraded)
    {
        _upgraded = true;
    }
    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
#ifdef ICE_CPP11_MAPPING
IceSSL::TransceiverI::closing(bool initiator, exception_ptr ex)
#else
IceSSL::TransceiverI::closing(bool initiator, const Ice::LocalException& ex)
#endif
{
    return _delegate->closing(initiator, ex);
}

void
IceSSL::TransceiverI::close()
{
    _delegate->close();
}

IceInternal::SocketOperation
IceSSL::TransceiverI::write(IceInternal::Buffer& buf)
{
    return _delegate->write(buf);
}

IceInternal::SocketOperation
IceSSL::TransceiverI::read(IceInternal::Buffer& buf)
{
    return _delegate->read(buf);
}

bool
IceSSL::TransceiverI::startWrite(IceInternal::Buffer& buf)
{
    if(_connected && !_upgraded)
    {
        StreamSocket^ stream = safe_cast<StreamSocket^>(_delegate->getNativeInfo()->fd());
        HostName^ host = ref new HostName(ref new String(IceUtil::stringToWstring(_host).c_str()));
        try
        {
            Windows::Foundation::IAsyncAction^ action = stream->UpgradeToSslAsync(SocketProtectionLevel::Tls12, host);
            getNativeInfo()->queueAction(IceInternal::SocketOperationWrite, action);
        }
        catch(Platform::Exception^ ex)
        {
            IceInternal::checkErrorCode(__FILE__, __LINE__, ex->HResult);
        }
        return true;
    }
    return _delegate->startWrite(buf);
}

void
IceSSL::TransceiverI::finishWrite(IceInternal::Buffer& buf)
{
    if(_connected && !_upgraded)
    {
        IceInternal::AsyncInfo* asyncInfo = getNativeInfo()->getAsyncInfo(IceInternal::SocketOperationWrite);
        if(asyncInfo->count == SOCKET_ERROR)
        {
            IceInternal::checkErrorCode(__FILE__, __LINE__, asyncInfo->error);
        }
        return;
    }
    _delegate->finishWrite(buf);
}

void
IceSSL::TransceiverI::startRead(IceInternal::Buffer& buf)
{
    _delegate->startRead(buf);
}

void
IceSSL::TransceiverI::finishRead(IceInternal::Buffer& buf)
{
    _delegate->finishRead(buf);
}

string
IceSSL::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
IceSSL::TransceiverI::toString() const
{
    return _delegate->toString();
}

string
IceSSL::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceSSL::TransceiverI::getInfo() const
{
    NativeConnectionInfoPtr info = ICE_MAKE_SHARED(NativeConnectionInfo);
    StreamSocket^ stream = safe_cast<StreamSocket^>(_delegate->getNativeInfo()->fd());
    info->nativeCerts.push_back(ICE_MAKE_SHARED(Certificate, stream->Information->ServerCertificate));
    info->certs.push_back(info->nativeCerts.back()->encode());
    for(auto iter = stream->Information->ServerIntermediateCertificates->First(); iter->HasCurrent; iter->MoveNext())
    {
        info->nativeCerts.push_back(ICE_MAKE_SHARED(Certificate, iter->Current));
        info->certs.push_back(info->nativeCerts.back()->encode());
    }
    info->adapterName = _adapterName;
    info->incoming = _incoming;
    info->underlying = _delegate->getInfo();
    return info;
}

void
IceSSL::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
IceSSL::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _delegate->setBufferSize(rcvSize, sndSize);
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance,
                                   const IceInternal::TransceiverPtr& delegate,
                                   const string& hostOrAdapterName,
                                   bool incoming) :
    _instance(instance),
    _engine(WinRTEnginePtr::dynamicCast(instance->engine())),
    _host(incoming ? "" : hostOrAdapterName),
    _adapterName(incoming ? hostOrAdapterName : ""),
    _incoming(incoming),
    _delegate(delegate),
    _connected(false),
    _upgraded(false)
{
}

IceSSL::TransceiverI::~TransceiverI()
{
}

#endif
