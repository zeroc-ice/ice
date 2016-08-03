// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/ConnectorI.h>
#include <IceSSL/Instance.h>

#include <IceSSL/OpenSSLTransceiverI.h>
#include <IceSSL/SecureTransportTransceiverI.h>
#include <IceSSL/SChannelTransceiverI.h>
#include <IceSSL/WinRTTransceiverI.h>

#include <IceSSL/EndpointI.h>
#include <IceSSL/Util.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/StreamSocket.h>
#include <Ice/LoggerUtil.h>
#include <Ice/NetworkProxy.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceInternal::TransceiverPtr
IceSSL::ConnectorI::connect()
{
    //
    // The plug-in may not be initialized.
    //
    if(!_instance->initialized())
    {
        PluginInitializationException ex(__FILE__, __LINE__);
        ex.reason = "IceSSL: plug-in is not initialized";
        throw ex;
    }

    return new TransceiverI(_instance, _delegate->connect(), _host, false);
}

Short
IceSSL::ConnectorI::type() const
{
    return _delegate->type();
}

string
IceSSL::ConnectorI::toString() const
{
    return _delegate->toString();
}

bool
IceSSL::ConnectorI::operator==(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_delegate != p->_delegate)
    {
        return false;
    }

    return true;
}

bool
IceSSL::ConnectorI::operator<(const IceInternal::Connector& r) const
{
    const ConnectorI* p = dynamic_cast<const ConnectorI*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(this == p)
    {
        return false;
    }

    if(_delegate < p->_delegate)
    {
        return true;
    }
    else if(p->_delegate < _delegate)
    {
        return false;
    }

    return false;
}

IceSSL::ConnectorI::ConnectorI(const InstancePtr& instance, const IceInternal::ConnectorPtr& del, const string& h) :
    _instance(instance), _delegate(del), _host(h)
{
}

IceSSL::ConnectorI::~ConnectorI()
{
}
