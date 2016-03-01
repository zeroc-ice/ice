// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/PropertiesAdminI.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Ice;

namespace
{

const char* traceCategory = "Admin.Properties";

}

namespace IceInternal
{

PropertiesAdminI::PropertiesAdminI(const PropertiesPtr& properties, const LoggerPtr& logger) :
    _properties(properties), _logger(logger)
{
}

string
PropertiesAdminI::getProperty(const string& name, const Current&)
{
    Lock sync(*this);
    return _properties->getProperty(name);
}

PropertyDict
PropertiesAdminI::getPropertiesForPrefix(const string& prefix, const Current&)
{
    Lock sync(*this);
    return _properties->getPropertiesForPrefix(prefix);
}

void
PropertiesAdminI::setProperties_async(const AMD_PropertiesAdmin_setPropertiesPtr& cb, const PropertyDict& props,
                                           const Current&)
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

    //
    // Send the response now so that we do not block the client during
    // the call to the update callback.
    //
    cb->ice_response();

    //
    // Copy the callbacks to allow callbacks to update the callbacks.
    //
    vector<PropertiesAdminUpdateCallbackPtr> callbacks = _updateCallbacks;
    if(!callbacks.empty())
    {
        PropertyDict changes = added;
        changes.insert(changed.begin(), changed.end());
        changes.insert(removed.begin(), removed.end());
        for(vector<PropertiesAdminUpdateCallbackPtr>::const_iterator p = callbacks.begin(); p != callbacks.end(); ++p)
        {
            try
            {
                (*p)->updated(changes);
            }
            catch(...)
            {
                // Ignore.
            }
        }
    }
}

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

}
