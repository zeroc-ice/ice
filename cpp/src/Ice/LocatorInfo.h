// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOCATOR_INFO_H
#define ICE_LOCATOR_INFO_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Time.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/LocatorF.h>
#include <Ice/ProxyF.h>
#include <Ice/EndpointIF.h>

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
    
    bool getAdapterEndpoints(const std::string&, int, ::std::vector<EndpointIPtr>&);
    void addAdapterEndpoints(const std::string&, const ::std::vector<EndpointIPtr>&);
    ::std::vector<EndpointIPtr> removeAdapterEndpoints(const std::string&);

    bool getProxy(const Ice::Identity&, int, Ice::ObjectPrx&);
    void addProxy(const Ice::Identity&, const Ice::ObjectPrx&);
    Ice::ObjectPrx removeProxy(const Ice::Identity&);
    
private:

    bool checkTTL(const IceUtil::Time&, int) const;

    std::map<std::string, std::pair<IceUtil::Time, std::vector<EndpointIPtr> > > _adapterEndpointsMap;
    std::map<Ice::Identity, std::pair<IceUtil::Time, Ice::ObjectPrx> > _objectMap;
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

    std::vector<EndpointIPtr> getEndpoints(const IndirectReferencePtr&, int, bool&);
    void clearCache(const IndirectReferencePtr&);
    void clearObjectCache(const IndirectReferencePtr&);

private:

    void trace(const std::string&, const IndirectReferencePtr&, const std::vector<EndpointIPtr>&);

    const Ice::LocatorPrx _locator;
    Ice::LocatorRegistryPrx _locatorRegistry;
    const LocatorTablePtr _table;
};

}

#endif
