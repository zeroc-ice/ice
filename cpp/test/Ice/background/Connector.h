//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_CONNECTOR_H
#define TEST_CONNECTOR_H

#include <Ice/Connector.h>
#include <Configuration.h>

class Connector : public IceInternal::Connector
{
public:

    IceInternal::TransceiverPtr connect();

    std::int16_t type() const;
    std::string toString() const;

    virtual bool operator==(const IceInternal::Connector&) const;
    virtual bool operator<(const IceInternal::Connector&) const;

    Connector(const IceInternal::ConnectorPtr& connector);

private:

    const IceInternal::ConnectorPtr _connector;
    const ConfigurationPtr _configuration;
};

#endif
