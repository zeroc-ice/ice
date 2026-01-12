// Copyright (c) ZeroC, Inc.

#include "Ice/NativePropertiesAdmin.h"
#include "Ice/Logger.h"
#include "Ice/LoggerUtil.h"
#include "Instance.h"

using namespace std;
using namespace Ice;

namespace
{
    const char* traceCategory = "Admin.Properties";
}

Ice::NativePropertiesAdmin::~NativePropertiesAdmin() = default; // Out of line to avoid weak vtable

Ice::NativePropertiesAdmin::NativePropertiesAdmin(PropertiesPtr properties, LoggerPtr logger)
    : _properties(std::move(properties)),
      _logger(std::move(logger))
{
}

string
Ice::NativePropertiesAdmin::getProperty(string name, const Current&)
{
    lock_guard lock{_mutex};
    return _properties->getProperty(name);
}

PropertyDict
Ice::NativePropertiesAdmin::getPropertiesForPrefix(string prefix, const Current&)
{
    lock_guard lock{_mutex};
    return _properties->getPropertiesForPrefix(prefix);
}

void
Ice::NativePropertiesAdmin::setProperties(PropertyDict props, const Current&)
{
    lock_guard lock{_mutex};

    PropertyDict old = _properties->getPropertiesForPrefix("");
    PropertyDict::const_iterator p;
    const int traceLevel = _properties->getIcePropertyAsInt("Ice.Trace.Admin.Properties");

    //
    // Compute the difference between the new property set and the existing property set:
    //
    // 1) Any properties in the new set that were not defined in the existing set.
    //
    // 2) Any properties that appear in both sets but with different values.
    //
    // 3) Any properties not present in the new set but present in the existing set.
    //    In other words, the property has been removed.
    //
    PropertyDict added, changed, removed;
    for (p = props.begin(); p != props.end(); ++p)
    {
        auto q = old.find(p->first);
        if (q == old.end())
        {
            if (!p->second.empty())
            {
                //
                // This property is new.
                //
                added.insert(*p);
            }
        }
        else
        {
            if (p->second != q->second)
            {
                if (p->second.empty())
                {
                    //
                    // This property was removed.
                    //
                    removed.insert(*p);
                }
                else
                {
                    //
                    // This property has changed.
                    //
                    changed.insert(*p);
                }
            }
        }
    }

    if (traceLevel > 0 && (!added.empty() || !changed.empty() || !removed.empty()))
    {
        Trace out(_logger, traceCategory);

        out << "Summary of property changes";

        if (!added.empty())
        {
            out << "\nNew properties:";
            for (p = added.begin(); p != added.end(); ++p)
            {
                out << "\n  " << p->first;
                if (traceLevel > 1)
                {
                    out << " = " << p->second;
                }
            }
        }

        if (!changed.empty())
        {
            out << "\nChanged properties:";
            for (p = changed.begin(); p != changed.end(); ++p)
            {
                out << "\n  " << p->first;
                if (traceLevel > 1)
                {
                    out << " = " << p->second << " (old value = " << _properties->getProperty(p->first) << ")";
                }
            }
        }

        if (!removed.empty())
        {
            out << "\nRemoved properties:";
            for (p = removed.begin(); p != removed.end(); ++p)
            {
                out << "\n  " << p->first;
            }
        }
    }

    //
    // Update the property set.
    //

    for (p = added.begin(); p != added.end(); ++p)
    {
        _properties->setProperty(p->first, p->second);
    }

    for (p = changed.begin(); p != changed.end(); ++p)
    {
        _properties->setProperty(p->first, p->second);
    }

    for (p = removed.begin(); p != removed.end(); ++p)
    {
        _properties->setProperty(p->first, "");
    }

    if (!_updateCallbacks.empty())
    {
        PropertyDict changes = added;
        changes.insert(changed.begin(), changed.end());
        changes.insert(removed.begin(), removed.end());

        // Copy callbacks to allow callbacks to update callbacks
        auto callbacks = _updateCallbacks;
        for (const auto& cb : callbacks)
        {
            cb(changes);
        }
    }
}

std::function<void()>
Ice::NativePropertiesAdmin::addUpdateCallback(std::function<void(const PropertyDict&)> cb)
{
    lock_guard lock{_mutex};

    auto p = _updateCallbacks.insert(_updateCallbacks.end(), std::move(cb));
    auto propertiesAdmin = shared_from_this();

    return [p, propertiesAdmin] { propertiesAdmin->removeUpdateCallback(p); };
}

void
Ice::NativePropertiesAdmin::removeUpdateCallback(std::list<std::function<void(const PropertyDict&)>>::iterator p)
{
    lock_guard lock{_mutex};
    _updateCallbacks.erase(p);
}
