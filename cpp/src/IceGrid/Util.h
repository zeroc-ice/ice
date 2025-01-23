// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_UTIL_H
#define ICEGRID_UTIL_H

#include "../Ice/Random.h"
#include "Ice/BuiltinSequences.h"
#include "Ice/StringUtil.h"
#include "IceGrid/Admin.h"
#include "IceGrid/Descriptor.h"
#include "IceGrid/Exception.h"

#include <functional>
#include <iterator>

namespace IceGrid
{
    template<typename T> std::insert_iterator<T> inline set_inserter(T& container)
    {
        return std::insert_iterator<T>(container, container.begin());
    }

    std::string toString(const std::vector<std::string>&, const std::string& = std::string(" "));
    std::string toString(std::exception_ptr);

    std::string getProperty(const PropertyDescriptorSeq&, const std::string&, const std::string& = std::string());
    int getPropertyAsInt(const PropertyDescriptorSeq&, const std::string&, int = 0);

    bool hasProperty(const PropertyDescriptorSeq&, const std::string&);
    PropertyDescriptor createProperty(const std::string&, const std::string& = std::string());
    std::string escapeProperty(const std::string&, bool = false);

    ObjectInfo toObjectInfo(const Ice::CommunicatorPtr&, const ObjectDescriptor&, const std::string&);

    void setupThreadPool(const Ice::PropertiesPtr&, const std::string&, int, int = 0, bool = false);

    int getMMVersion(const std::string&);

    int secondsToInt(const std::chrono::seconds&);

    void createDirectory(const std::string&);
    Ice::StringSeq readDirectory(const std::string&);
    void remove(const std::string&);
    void removeRecursive(const std::string&);
    std::string simplify(const std::string&);

    inline void forEachCommunicator(
        const std::shared_ptr<CommunicatorDescriptor>& descriptor,
        const std::function<void(const std::shared_ptr<CommunicatorDescriptor>&)>& callback)
    {
        callback(descriptor);
        auto iceBox = std::dynamic_pointer_cast<IceBoxDescriptor>(descriptor);
        if (iceBox)
        {
            for (const auto& service : iceBox->services)
            {
                forEachCommunicator(service.descriptor, callback);
            }
        }
    }

    inline void forEachCommunicator(
        const std::shared_ptr<CommunicatorDescriptor>& oldDescriptor,
        const std::shared_ptr<CommunicatorDescriptor>& newDescriptor,
        const std::function<
            void(const std::shared_ptr<CommunicatorDescriptor>&, const std::shared_ptr<CommunicatorDescriptor>&)>&
            callback)
    {
        callback(oldDescriptor, newDescriptor);

        auto oldIceBox = std::dynamic_pointer_cast<IceBoxDescriptor>(oldDescriptor);
        auto newIceBox = std::dynamic_pointer_cast<IceBoxDescriptor>(newDescriptor);

        if (oldIceBox && !newIceBox)
        {
            for (const auto& service : oldIceBox->services)
            {
                callback(service.descriptor, nullptr);
            }
        }
        else if (!oldIceBox && newIceBox)
        {
            for (const auto& service : newIceBox->services)
            {
                callback(nullptr, service.descriptor);
            }
        }
        else if (oldIceBox && newIceBox)
        {
            for (const auto& oldService : oldIceBox->services)
            {
                ServiceInstanceDescriptorSeq::const_iterator q;
                for (q = newIceBox->services.begin(); q != newIceBox->services.end(); ++q)
                {
                    if (oldService.descriptor->name == q->descriptor->name)
                    {
                        callback(oldService.descriptor, q->descriptor);
                        break;
                    }
                }
                if (q == newIceBox->services.end())
                {
                    callback(oldService.descriptor, nullptr);
                }
            }
            for (const auto& newService : newIceBox->services)
            {
                ServiceInstanceDescriptorSeq::const_iterator q;
                for (q = oldIceBox->services.begin(); q != oldIceBox->services.end(); ++q)
                {
                    if (newService.descriptor->name == q->descriptor->name)
                    {
                        break;
                    }
                }
                if (q == oldIceBox->services.end())
                {
                    callback(nullptr, newService.descriptor);
                }
            }
        }
    }

    template<class T> std::vector<std::string> inline getMatchingKeys(const T& m, const std::string& expression)
    {
        std::vector<std::string> keys;
        for (const auto& entry : m)
        {
            if (expression.empty() || IceInternal::match(entry.first, expression, true))
            {
                keys.push_back(entry.first);
            }
        }
        return keys;
    }

};

#endif
