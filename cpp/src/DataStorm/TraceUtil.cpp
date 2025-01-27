// Copyright (c) ZeroC, Inc.

#include "TraceUtil.h"

using namespace std;
using namespace DataStormI;
using namespace Ice;

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

TraceLevels::TraceLevels(const PropertiesPtr& properties, LoggerPtr logger)
    : topic(properties->getIcePropertyAsInt("DataStorm.Trace.Topic")),
      data(properties->getIcePropertyAsInt("DataStorm.Trace.Data")),
      session(properties->getIcePropertyAsInt("DataStorm.Trace.Session")),
      logger(std::move(logger))
{
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

// Our custom implementation of various operator<< for structs defined in Contract.ice

namespace DataStormContract
{
    inline std::string valueIdToString(std::int64_t valueId)
    {
        if (valueId < 0)
        {
            return "f" + std::to_string(-valueId);
        }
        else
        {
            return "k" + std::to_string(valueId);
        }
    }

    ostream& operator<<(ostream& os, const ElementInfo& info)
    {
        os << valueIdToString(info.id);
        return os;
    }

    ostream& operator<<(ostream& os, const ElementData& data)
    {
        os << 'e' << data.id;
        if (data.config && data.config->facet)
        {
            os << ':' << *data.config->facet;
        }
        return os;
    }

    ostream& operator<<(ostream& os, const ElementSpec& spec)
    {
        for (auto q = spec.elements.begin(); q != spec.elements.end(); ++q)
        {
            if (q != spec.elements.begin())
            {
                os << ',';
            }
            os << *q << ':' << valueIdToString(spec.id) << ":pv" << valueIdToString(spec.peerId);
        }
        return os;
    }

    ostream& operator<<(ostream& os, const ElementDataAck& data)
    {
        os << 'e' << data.id << ":pe" << data.peerId;
        if (data.config && data.config->facet)
        {
            os << ':' << *data.config->facet;
        }
        os << ":sz" << data.samples.size();
        return os;
    }

    ostream& operator<<(ostream& os, const ElementSpecAck& spec)
    {
        for (auto q = spec.elements.begin(); q != spec.elements.end(); ++q)
        {
            if (q != spec.elements.begin())
            {
                os << ',';
            }
            os << *q << ':' << valueIdToString(spec.id) << ":pv" << valueIdToString(spec.peerId);
        }
        return os;
    }

    ostream& operator<<(ostream& os, const TopicInfo& info)
    {
        os << "[" << info.ids << "]:" << info.name;
        return os;
    }

    ostream& operator<<(ostream& os, const TopicSpec& info)
    {
        os << info.id << ':' << info.name << ":[" << info.elements << "]";
        return os;
    }

    ostream& operator<<(ostream& os, const DataSamples& samples)
    {
        os << 'e' << samples.id << ":sz" << samples.samples.size();
        return os;
    }
}
