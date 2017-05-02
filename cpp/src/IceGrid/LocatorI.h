// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_LOCATOR_I_H
#define ICE_GRID_LOCATOR_I_H

#include <IceGrid/Internal.h>
#include <IceGrid/Registry.h>

#include <set>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class WellKnownObjectsManager;
typedef IceUtil::Handle<WellKnownObjectsManager> WellKnownObjectsManagerPtr;

class LocatorI;
typedef IceUtil::Handle<LocatorI> LocatorIPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

struct LocatorAdapterInfo;
typedef std::vector<LocatorAdapterInfo> LocatorAdapterInfoSeq;

class LocatorI : public Locator, public IceUtil::Mutex
{
public:

    class Request : public virtual IceUtil::Shared
    {
    public:

        virtual void execute() = 0;
        virtual void activating(const std::string&) = 0;
        virtual void response(const std::string&, const Ice::ObjectPrx&) = 0;
        virtual void exception(const std::string&, const Ice::Exception&) = 0;
    };
    typedef IceUtil::Handle<Request> RequestPtr;

    LocatorI(const Ice::CommunicatorPtr&, const DatabasePtr&, const WellKnownObjectsManagerPtr&, const RegistryPrx&,
             const QueryPrx&);

    virtual void findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr&, const Ice::Identity&,
                                      const Ice::Current&) const;

    virtual void findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr&, const ::std::string&,
                                       const Ice::Current&) const;

    virtual Ice::LocatorRegistryPrx getRegistry(const Ice::Current&) const;
    virtual RegistryPrx getLocalRegistry(const Ice::Current&) const;
    virtual QueryPrx getLocalQuery(const Ice::Current&) const;

    const Ice::CommunicatorPtr& getCommunicator() const;
    const TraceLevelsPtr& getTraceLevels() const;

    bool getDirectProxy(const LocatorAdapterInfo&, const RequestPtr&);
    void getDirectProxyResponse(const LocatorAdapterInfo&, const Ice::ObjectPrx&);
    void getDirectProxyException(const LocatorAdapterInfo&, const Ice::Exception&);

protected:

    const Ice::CommunicatorPtr _communicator;
    const DatabasePtr _database;
    const WellKnownObjectsManagerPtr _wellKnownObjects;
    const RegistryPrx _localRegistry;
    const QueryPrx _localQuery;

    typedef std::vector<RequestPtr> PendingRequests;
    typedef std::map<std::string, PendingRequests> PendingRequestsMap;
    PendingRequestsMap _pendingRequests;
    std::set<std::string> _activating;
};

}

#endif
