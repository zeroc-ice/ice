// Copyright (c) ZeroC, Inc.

#ifndef ICE_CONNECTOR_H
#define ICE_CONNECTOR_H

#include "ConnectorF.h"
#include "TransceiverF.h"

#include <cstdint>
#include <string>

#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace IceInternal
{
    class Connector
    {
    public:
        virtual ~Connector() = default;

        virtual TransceiverPtr connect() = 0;

        [[nodiscard]] virtual std::int16_t type() const = 0;
        [[nodiscard]] virtual std::string toString() const = 0;

        virtual bool operator==(const Connector&) const = 0;
        virtual bool operator<(const Connector&) const = 0;
    };
}

#ifdef __clang__
#    pragma clang diagnostic pop
#endif

#endif
