// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOCATOR_INFO_H
#define ICEE_LOCATOR_INFO_H

#include <IceE/Config.h>

#ifdef ICEE_HAS_LOCATOR

#include <IceE/LocatorInfoF.h>
#include <IceE/LocatorF.h>
#include <IceE/ProxyF.h>
#include <IceE/EndpointF.h>

#include <IceE/Shared.h>
#include <IceE/Mutex.h>

namespace IceInternal
{

class LocatorManager : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    LocatorManager();

    void destroy();

    //
    // Returns locator info for a given locator. Automatically creates
    // the locator info if it doesn't exist yet.
    //
    LocatorInfoPtr get(const Ice::LocatorPrx&);

private:

    std::map<Ice::LocatorPrx, LocatorInfoPtr> _table;
    std::map<Ice::LocatorPrx, LocatorInfoPtr>::iterator _tableHint;

    std::map<Ice::Identity, LocatorTablePtr> _locatorTables;
};

class LocatorTable : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    LocatorTable();

    void clear();
    
    bool getAdapterEndpoints(const std::string&, ::std::vector<EndpointPtr>&) const;
    void addAdapterEndpoints(const std::string&, const ::std::vector<EndpointPtr>&);
    ::std::vector<EndpointPtr> removeAdapterEndpoints(const std::string&);

    bool getProxy(const Ice::Identity&, Ice::ObjectPrx&) const;
    void addProxy(const Ice::Identity&, const Ice::ObjectPrx&);
    Ice::ObjectPrx removeProxy(const Ice::Identity&);
    
private:

    std::map<std::string, std::vector<EndpointPtr> > _adapterEndpointsMap;
    std::map<Ice::Identity, Ice::ObjectPrx > _objectMap;
};

class LocatorInfo : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    LocatorInfo(const Ice::LocatorPrx&, const LocatorTablePtr&);

    void destroy();

    bool operator==(const LocatorInfo&) const;
    bool operator!=(const LocatorInfo&) const;
    bool operator<(const LocatorInfo&) const;

    Ice::LocatorPrx getLocator() const;
    Ice::LocatorRegistryPrx getLocatorRegistry();

    std::vector<EndpointPtr> getEndpoints(const IndirectReferencePtr&, bool&);
    void clearCache(const IndirectReferencePtr&);
    void clearObjectCache(const IndirectReferencePtr&);

private:

    void trace(const std::string&, const IndirectReferencePtr&, const std::vector<EndpointPtr>&);

    const Ice::LocatorPrx _locator;
    Ice::LocatorRegistryPrx _locatorRegistry;
    const LocatorTablePtr _table;
};

}

#endif

#endif
