// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_CONNECTOR_H
#define TEST_CONNECTOR_H

#include <Ice/Connector.h>
#include <Configuration.h>

class Connector : public IceInternal::Connector
{
public:

    IceInternal::TransceiverPtr connect();

    Ice::Short type() const;
    std::string toString() const;

    virtual bool operator==(const IceInternal::Connector&) const;
    virtual bool operator<(const IceInternal::Connector&) const;

    Connector(const IceInternal::ConnectorPtr& connector);

private:

    const IceInternal::ConnectorPtr _connector;
    const ConfigurationPtr _configuration;
};

#endif
