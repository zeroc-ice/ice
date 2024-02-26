//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTOR_H
#define ICE_CONNECTOR_H

#include <Ice/Config.h>
#include <Ice/ConnectorF.h>
#include <Ice/TransceiverF.h>

namespace IceInternal
{

class ICE_API Connector
{
public:

    virtual ~Connector();

    virtual TransceiverPtr connect() = 0;

    virtual std::int16_t type() const = 0;
    virtual std::string toString() const = 0;

    virtual bool operator==(const Connector&) const = 0;
    virtual bool operator<(const Connector&) const = 0;
};

}

#endif
