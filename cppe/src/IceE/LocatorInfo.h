// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOCATOR_INFO_H
#define ICEE_LOCATOR_INFO_H

#include <IceE/Config.h>

#ifndef ICEE_NO_LOCATOR

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/LocatorInfoF.h>
#include <IceE/LocatorF.h>
#include <IceE/ProxyF.h>
#include <IceE/EndpointF.h>

namespace IceEInternal
{

class LocatorManager : public IceE::Shared, public IceE::Mutex
{
public:

    LocatorManager();

    void destroy();

    //
    // Returns locator info for a given locator. Automatically creates
    // the locator info if it doesn't exist yet.
    //
    LocatorInfoPtr get(const IceE::LocatorPrx&);

private:

    std::map<IceE::LocatorPrx, LocatorInfoPtr> _table;
    std::map<IceE::LocatorPrx, LocatorInfoPtr>::iterator _tableHint;

    std::map<IceE::Identity, LocatorTablePtr> _locatorTables;
};

class LocatorTable : public IceE::Shared, public IceE::Mutex
{
public:

    LocatorTable();

    void clear();
    
    bool getAdapterEndpoints(const std::string&, ::std::vector<EndpointPtr>&) const;
    void addAdapterEndpoints(const std::string&, const ::std::vector<EndpointPtr>&);
    ::std::vector<EndpointPtr> removeAdapterEndpoints(const std::string&);

    bool getProxy(const IceE::Identity&, IceE::ObjectPrx&) const;
    void addProxy(const IceE::Identity&, const IceE::ObjectPrx&);
    IceE::ObjectPrx removeProxy(const IceE::Identity&);
    
private:

    std::map<std::string, std::vector<EndpointPtr> > _adapterEndpointsMap;
    std::map<IceE::Identity, IceE::ObjectPrx > _objectMap;
};

class LocatorInfo : public IceE::Shared, public IceE::Mutex
{
public:

    LocatorInfo(const IceE::LocatorPrx&, const LocatorTablePtr&);

    void destroy();

    bool operator==(const LocatorInfo&) const;
    bool operator!=(const LocatorInfo&) const;
    bool operator<(const LocatorInfo&) const;

    IceE::LocatorPrx getLocator() const;
    IceE::LocatorRegistryPrx getLocatorRegistry();

    std::vector<EndpointPtr> getEndpoints(const IndirectReferencePtr&, bool&);
    void clearCache(const IndirectReferencePtr&);
    void clearObjectCache(const IndirectReferencePtr&);

private:

    void trace(const std::string&, const IndirectReferencePtr&, const std::vector<EndpointPtr>&);

    const IceE::LocatorPrx _locator;
    IceE::LocatorRegistryPrx _locatorRegistry;
    const LocatorTablePtr _table;
};

}

#endif

#endif
