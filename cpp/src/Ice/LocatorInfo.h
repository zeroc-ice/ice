// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_LOCATOR_INFO_H
#define ICE_LOCATOR_INFO_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/LocatorF.h>
#include <Ice/ProxyF.h>
#include <Ice/EndpointF.h>

namespace IceInternal
{

class LocatorManager : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    LocatorManager();

    void destroy();

    //
    // Returns locator info for a given locator. Automatically creates
    // the locator info if it doesn't exist yet.
    //
    LocatorInfoPtr get(const ::Ice::LocatorPrx&);

private:

    std::map< ::Ice::LocatorPrx, LocatorInfoPtr> _table;
    std::map< ::Ice::LocatorPrx, LocatorInfoPtr>::iterator _tableHint;

    std::map< ::Ice::Identity, LocatorTablePtr> _locatorTables;
};

class LocatorTable : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    LocatorTable();

    void clear();
    
    bool getAdapterEndpoints(const std::string&, ::std::vector<EndpointPtr>&) const;
    void addAdapterEndpoints(const std::string&, const ::std::vector<EndpointPtr>&);
    ::std::vector<EndpointPtr> removeAdapterEndpoints(const std::string&);

    bool getProxy(const ::Ice::Identity&, ::Ice::ObjectPrx&) const;
    void addProxy(const ::Ice::Identity&, const Ice::ObjectPrx&);
    Ice::ObjectPrx removeProxy(const ::Ice::Identity&);
    
private:

    std::map<std::string, std::vector<EndpointPtr> > _adapterEndpointsMap;
    std::map<Ice::Identity, Ice::ObjectPrx > _objectMap;
};

class LocatorInfo : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    LocatorInfo(const ::Ice::LocatorPrx&, const LocatorTablePtr&);

    void destroy();

    bool operator==(const LocatorInfo&) const;
    bool operator!=(const LocatorInfo&) const;
    bool operator<(const LocatorInfo&) const;

    ::Ice::LocatorPrx getLocator() const;
    ::Ice::LocatorRegistryPrx getLocatorRegistry();

    std::vector<EndpointPtr> getEndpoints(const ReferencePtr&, bool&);
    void clearCache(const ReferencePtr&);
    void clearObjectCache(const ReferencePtr&);

private:

    void trace(const std::string&, const ReferencePtr&, const std::vector<EndpointPtr>&);

    ::Ice::LocatorPrx _locator; // Immutable.
    ::Ice::LocatorRegistryPrx _locatorRegistry;
    LocatorTablePtr _table; // Immutable.
};

}

#endif
