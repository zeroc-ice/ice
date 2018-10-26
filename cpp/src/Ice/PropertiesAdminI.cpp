// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/PropertiesAdminI.h>
#include <Ice/Instance.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>
#include <Ice/ThreadPool.h>

using namespace std;
using namespace Ice;

namespace
{

const char* traceCategory = "Admin.Properties";

}

#ifndef ICE_CPP11_MAPPING
PropertiesAdminUpdateCallback::~PropertiesAdminUpdateCallback()
{
    // Out of line to avoid weak vtable
}
#endif

NativePropertiesAdmin::~NativePropertiesAdmin()
{
    // Out of line to avoid weak vtable
}

namespace IceInternal
{

PropertiesAdminI::PropertiesAdminI(const InstancePtr& instance) :
    _properties(instance->initializationData().properties),
    _logger(instance->initializationData().logger)
{
}

string
#ifdef ICE_CPP11_MAPPING
PropertiesAdminI::getProperty(string name, const Current&)
#else
PropertiesAdminI::getProperty(const string& name, const Current&)
#endif
{
    Lock sync(*this);
    return _properties->getProperty(name);
}

PropertyDict
#ifdef ICE_CPP11_MAPPING
PropertiesAdminI::getPropertiesForPrefix(string prefix, const Current&)
#else
PropertiesAdminI::getPropertiesForPrefix(const string& prefix, const Current&)
#endif
{
    Lock sync(*this);
    return _properties->getPropertiesForPrefix(prefix);
}

void
#ifdef ICE_CPP11_MAPPING
PropertiesAdminI::setProperties(PropertyDict props, const Current&)
#else
PropertiesAdminI::setProperties(const PropertyDict& props, const Current&)
#endif
{
    Lock sync(*this);

    PropertyDict old = _properties->getPropertiesForPrefix("");
    PropertyDict::const_iterator p;
    const int traceLevel = _properties->getPropertyAsInt("Ice.Trace.Admin.Properties");

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
    for(p = props.begin(); p != props.end(); ++p)
    {
        PropertyDict::iterator q = old.find(p->first);
        if(q == old.end())
        {
            if(!p->second.empty())
            {
                //
                // This property is new.
                //
                added.insert(*p);
            }
        }
        else
        {
            if(p->second != q->second)
            {
                if(p->second.empty())
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

    if(traceLevel > 0 && (!added.empty() || !changed.empty() || !removed.empty()))
    {
        Trace out(_logger, traceCategory);

        out << "Summary of property changes";

        if(!added.empty())
        {
            out << "\nNew properties:";
            for(p = added.begin(); p != added.end(); ++p)
            {
                out << "\n  " << p->first;
                if(traceLevel > 1)
                {
                    out << " = " << p->second;
                }
            }
        }

        if(!changed.empty())
        {
            out << "\nChanged properties:";
            for(p = changed.begin(); p != changed.end(); ++p)
            {
                out << "\n  " << p->first;
                if(traceLevel > 1)
                {
                    out << " = " << p->second << " (old value = " << _properties->getProperty(p->first) << ")";
                }
            }
        }

        if(!removed.empty())
        {
            out << "\nRemoved properties:";
            for(p = removed.begin(); p != removed.end(); ++p)
            {
                out << "\n  " << p->first;
            }
        }
    }

    //
    // Update the property set.
    //

    for(p = added.begin(); p != added.end(); ++p)
    {
        _properties->setProperty(p->first, p->second);
    }

    for(p = changed.begin(); p != changed.end(); ++p)
    {
        _properties->setProperty(p->first, p->second);
    }

    for(p = removed.begin(); p != removed.end(); ++p)
    {
        _properties->setProperty(p->first, "");
    }

    if(!_updateCallbacks.empty())
    {
        PropertyDict changes = added;
        changes.insert(changed.begin(), changed.end());
        changes.insert(removed.begin(), removed.end());

        // Copy callbacks to allow callbacks to update callbacks
#ifdef ICE_CPP11_MAPPING
        auto callbacks = _updateCallbacks;
        for(const auto& cb : callbacks)
#else
        vector<PropertiesAdminUpdateCallbackPtr> callbacks = _updateCallbacks;
        for(vector<PropertiesAdminUpdateCallbackPtr>::const_iterator q = callbacks.begin(); q != callbacks.end(); ++q)
#endif
        {
            try
            {
#ifdef ICE_CPP11_MAPPING
                cb(changes);
#else
                (*q)->updated(changes);
#endif
            }
            catch(const std::exception& ex)
            {
                if(_properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                {
                    Warning out(_logger);
                    out << "properties admin update callback raised unexpected exception:\n" << ex;
                }
            }
            catch(...)
            {
                if(_properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                {
                    Warning out(_logger);
                    out << "properties admin update callback raised unexpected exception:\nunknown c++ exception";
                }
            }
        }
    }
}

#ifdef ICE_CPP11_MAPPING

std::function<void()>
PropertiesAdminI::addUpdateCallback(std::function<void(const Ice::PropertyDict&)> cb)
{
    Lock sync(*this);

    auto p = _updateCallbacks.insert(_updateCallbacks.end(), std::move(cb));
    auto propertiesAdmin = shared_from_this();

    return [p, propertiesAdmin] { propertiesAdmin->removeUpdateCallback(p); };
}

void
PropertiesAdminI::removeUpdateCallback(std::list<std::function<void(const Ice::PropertyDict&)>>::iterator p)
{
    Lock sync(*this);
    _updateCallbacks.erase(p);
}

#else

void
PropertiesAdminI::addUpdateCallback(const PropertiesAdminUpdateCallbackPtr& cb)
{
    Lock sync(*this);
    _updateCallbacks.push_back(cb);
}

void
PropertiesAdminI::removeUpdateCallback(const PropertiesAdminUpdateCallbackPtr& cb)
{
    Lock sync(*this);
    _updateCallbacks.erase(remove(_updateCallbacks.begin(), _updateCallbacks.end(), cb), _updateCallbacks.end());
}

#endif

}
