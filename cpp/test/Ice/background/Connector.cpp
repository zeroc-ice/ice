// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Connector.h>
#include <Transceiver.h>
#include <EndpointI.h>

using namespace std;

IceInternal::TransceiverPtr
Connector::connect()
{
    _configuration->checkConnectException();
    return new Transceiver(_connector->connect());
}

Ice::Short
Connector::type() const
{
    return (Ice::Short)(EndpointI::TYPE_BASE + _connector->type());
}

string
Connector::toString() const
{
    return _connector->toString();
}

bool
Connector::operator==(const IceInternal::Connector& r) const
{
    const Connector* p = dynamic_cast<const Connector*>(&r);
    if(!p)
    {
        return false;
    }

    return *_connector == *p->_connector;
}

bool
Connector::operator!=(const IceInternal::Connector& r) const
{
    return !operator==(r);
}

bool
Connector::operator<(const IceInternal::Connector& r) const
{
    const Connector* p = dynamic_cast<const Connector*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    return *_connector < *p->_connector;
}

Connector::Connector(const IceInternal::ConnectorPtr& connector) :
    _connector(connector),
    _configuration(Configuration::getInstance())
{
}
