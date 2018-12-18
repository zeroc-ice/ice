// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceSSL/AcceptorI.h>
#include <IceSSL/EndpointI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>

#include <IceSSL/Util.h>

#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceUtil::Shared* IceSSL::upCast(AcceptorI* p) { return p; }

IceInternal::NativeInfoPtr
IceSSL::AcceptorI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
IceInternal::AsyncInfo*
IceSSL::AcceptorI::getAsyncInfo(IceInternal::SocketOperation status)
{
    return _delegate->getNativeInfo()->getAsyncInfo(status);
}
#endif

void
IceSSL::AcceptorI::close()
{
    _delegate->close();
}

IceInternal::EndpointIPtr
IceSSL::AcceptorI::listen()
{
    _endpoint = _endpoint->endpoint(_delegate->listen());
    return _endpoint;
}

#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
void
IceSSL::AcceptorI::startAccept()
{
    _delegate->startAccept();

}

void
IceSSL::AcceptorI::finishAccept()
{
    _delegate->finishAccept();
}
#endif

IceInternal::TransceiverPtr
IceSSL::AcceptorI::accept()
{
    //
    // The plug-in may not be initialized.
    //
    if(!_instance->initialized())
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: plug-in is not initialized");
    }

    return _instance->engine()->createTransceiver(_instance, _delegate->accept(), _adapterName, true);
}

string
IceSSL::AcceptorI::protocol() const
{
    return _delegate->protocol();
}

string
IceSSL::AcceptorI::toString() const
{
    return _delegate->toString();
}

string
IceSSL::AcceptorI::toDetailedString() const
{
    return _delegate->toDetailedString();
}

IceSSL::AcceptorI::AcceptorI(const EndpointIPtr& endpoint, const InstancePtr& instance,
                             const IceInternal::AcceptorPtr& del, const string& adapterName) :
    _endpoint(endpoint),
    _instance(instance),
    _delegate(del),
    _adapterName(adapterName)
{
}

IceSSL::AcceptorI::~AcceptorI()
{
}
