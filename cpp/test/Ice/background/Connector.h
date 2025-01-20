// Copyright (c) ZeroC, Inc.

#ifndef TEST_CONNECTOR_H
#define TEST_CONNECTOR_H

#include "Configuration.h"
#include "Ice/Connector.h"

class Connector : public IceInternal::Connector
{
public:
    IceInternal::TransceiverPtr connect() override;

    [[nodiscard]] std::int16_t type() const override;
    [[nodiscard]] std::string toString() const override;

    bool operator==(const IceInternal::Connector&) const override;
    bool operator<(const IceInternal::Connector&) const override;

    Connector(IceInternal::ConnectorPtr connector);

private:
    const IceInternal::ConnectorPtr _connector;
    const ConfigurationPtr _configuration;
};

#endif
