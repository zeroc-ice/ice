// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOCATOR_INFO_H
#define ICE_LOCATOR_INFO_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/Locator.h>
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

    std::map< ::Ice::Identity, LocatorAdapterTablePtr> _adapterTables;
};

class LocatorAdapterTable : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    LocatorAdapterTable();

    void clear();
    
    bool get(const std::string&, ::std::vector<EndpointPtr>&) const;
    void add(const std::string&, const ::std::vector<EndpointPtr>&);
    ::std::vector<EndpointPtr> remove(const std::string&);
    
private:

    std::map<std::string, std::vector<EndpointPtr> > _adapterEndpointsMap;
};

class LocatorInfo : public ::IceUtil::Shared, public ::IceUtil::Mutex
{
public:

    LocatorInfo(const ::Ice::LocatorPrx&, const LocatorAdapterTablePtr&);

    void destroy();

    bool operator==(const LocatorInfo&) const;
    bool operator!=(const LocatorInfo&) const;
    bool operator<(const LocatorInfo&) const;

    ::Ice::LocatorPrx getLocator() const;
    ::Ice::LocatorRegistryPrx getLocatorRegistry();

    std::vector<EndpointPtr> getEndpoints(const ReferencePtr&, bool&);
    void clearCache(const ReferencePtr&);

private:

    ::Ice::LocatorPrx _locator; // Immutable.
    ::Ice::LocatorRegistryPrx _locatorRegistry;
    LocatorAdapterTablePtr _adapterTable; // Immutable.
};

}

#endif
