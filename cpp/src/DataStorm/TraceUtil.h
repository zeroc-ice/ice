//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_TRACE_UTIL_H
#define DATASTORM_TRACE_UTIL_H

#include "DataElementI.h"
#include "DataStorm/InternalI.h"
#include "Ice/Ice.h"
#include "SessionI.h"
#include "TopicI.h"

namespace std
{
    template<typename T> inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& p)
    {
        if (!p.empty())
        {
            for (auto q = p.begin(); q != p.end(); ++q)
            {
                if (q != p.begin())
                {
                    os << ", ";
                }
                os << *q;
            }
        }
        return os;
    }

    template<typename K, typename V> inline std::ostream& operator<<(std::ostream& os, const std::map<K, V>& p)
    {
        if (!p.empty())
        {
            for (auto q = p.begin(); q != p.end(); ++q)
            {
                if (q != p.begin())
                {
                    os << ", ";
                }
                os << q->first << "=" << q->second;
            }
        }
        return os;
    }
}

namespace Ice
{
    inline std::ostream& operator<<(std::ostream& os, const Ice::Identity& id)
    {
        return os << Ice::identityToString(id);
    }
}

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

    inline std::ostream& operator<<(std::ostream& os, const ElementInfo& info)
    {
        os << valueIdToString(info.id);
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const ElementData& data)
    {
        os << 'e' << data.id;
        if (data.config && data.config->facet)
        {
            os << ':' << *data.config->facet;
        }
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const ElementSpec& spec)
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

    inline std::ostream& operator<<(std::ostream& os, const ElementDataAck& data)
    {
        os << 'e' << data.id << ":pe" << data.peerId;
        if (data.config && data.config->facet)
        {
            os << ':' << *data.config->facet;
        }
        os << ":sz" << data.samples.size();
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const ElementSpecAck& spec)
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

    inline std::ostream& operator<<(std::ostream& os, const TopicInfo& info)
    {
        os << "[" << info.ids << "]:" << info.name;
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const TopicSpec& info)
    {
        os << info.id << ':' << info.name << ":[" << info.elements << "]";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const DataSamples& samples)
    {
        os << 'e' << samples.id << ":sz" << samples.samples.size();
        return os;
    }
}

namespace DataStormI
{
    template<typename T, typename ::std::enable_if<::std::is_base_of<DataStormI::Element, T>::value>::type* = nullptr>
    inline std::ostream& operator<<(std::ostream& os, const std::shared_ptr<T>& p)
    {
        os << (p ? p->toString() : "");
        return os;
    }

    template<
        typename T,
        typename ::std::enable_if<::std::is_base_of<DataStormI::DataElementI, T>::value>::type* = nullptr>
    inline std::ostream& operator<<(std::ostream& os, T* element)
    {
        os << (element ? element->toString() : "<null>");
        return os;
    }

    template<
        typename T,
        typename ::std::enable_if<::std::is_base_of<DataStormI::DataElementI, T>::value>::type* = nullptr>
    inline std::ostream& operator<<(std::ostream& os, const std::shared_ptr<T>& element)
    {
        os << element.get();
        return os;
    }

    template<typename T, typename ::std::enable_if<::std::is_base_of<DataStormI::SessionI, T>::value>::type* = nullptr>
    inline std::ostream& operator<<(std::ostream& os, T* session)
    {
        if (session)
        {
            os << Ice::identityToString(session->getNode()->ice_getIdentity());
        }
        else
        {
            os << "<null>";
        }
        return os;
    }

    template<
        typename T,
        typename ::std::enable_if<
            ::std::is_base_of<DataStormI::TopicI, typename std::remove_pointer<T>::type>::value>::type* = nullptr>
    inline std::ostream& operator<<(std::ostream& os, T topic)
    {
        if (topic)
        {
            os << topic->getId() << ":" << topic->getName();
        }
        else
        {
            os << "<null>";
        }
        return os;
    }

    template<typename T, typename ::std::enable_if<::std::is_base_of<DataStormI::TopicI, T>::value>::type* = nullptr>
    inline std::ostream& operator<<(std::ostream& os, const std::shared_ptr<T>& topic)
    {
        os << topic.get();
        return os;
    }

    class TraceLevels
    {
    public:
        TraceLevels(const Ice::PropertiesPtr&, Ice::LoggerPtr);

        const int topic;
        const char* topicCat;

        const int data;
        const char* dataCat;

        const int session;
        const char* sessionCat;

        const Ice::LoggerPtr logger;
    };

    class Trace : public Ice::Trace
    {
    public:
        Trace(std::shared_ptr<TraceLevels> traceLevels, const std::string& category)
            : Ice::Trace(traceLevels->logger, category)
        {
        }
    };

    class Warning : public Ice::Warning
    {
    public:
        Warning(std::shared_ptr<TraceLevels> traceLevels) : Ice::Warning(traceLevels->logger) {}
    };
}
#endif
