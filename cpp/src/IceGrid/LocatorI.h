// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_LOCATOR_I_H
#define ICE_GRID_LOCATOR_I_H

#include <IceGrid/Internal.h>
#include <IceGrid/Locator.h>

#include <set>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;
    
class LocatorI;
typedef IceUtil::Handle<LocatorI> LocatorIPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

struct LocatorAdapterInfo;
typedef std::vector<LocatorAdapterInfo> LocatorAdapterInfoSeq;

class LocatorI : public Locator, public IceUtil::Mutex
{
public:

    class Request : public IceUtil::Mutex, public IceUtil::Shared
    {
    public:

        Request(const Ice::AMD_Locator_findAdapterByIdPtr&, const LocatorIPtr&, const std::string&, bool, bool,
                const LocatorAdapterInfoSeq&, int);

        void execute();
        void response(const std::string&, const Ice::ObjectPrx&);
        void activate(const std::string&);
        void exception(const std::string&, const Ice::Exception&); 

        virtual bool
        operator<(const Request& r) const
        {
            return this < &r;
        }

    private:

        void requestAdapter(const LocatorAdapterInfo&);
        void sendResponse();

        const Ice::AMD_Locator_findAdapterByIdPtr _amdCB;
        const LocatorIPtr _locator;
        const std::string _id;
        const bool _replicaGroup;
        const bool _roundRobin;
        LocatorAdapterInfoSeq _adapters;
        const TraceLevelsPtr _traceLevels;
        unsigned int _count;
        LocatorAdapterInfoSeq::const_iterator _lastAdapter;
        std::map<std::string, Ice::ObjectPrx> _proxies;
        std::auto_ptr<Ice::Exception> _exception;
        std::set<std::string> _activating;
    };
    typedef IceUtil::Handle<Request> RequestPtr;

    LocatorI(const Ice::CommunicatorPtr&, const DatabasePtr&, const Ice::LocatorRegistryPrx&, const RegistryPrx&,
             const QueryPrx&);

    virtual void findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr&, const Ice::Identity&, 
                                      const Ice::Current&) const;

    virtual void findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr&, const ::std::string&, 
                                       const Ice::Current& = Ice::Current()) const;

    virtual Ice::LocatorRegistryPrx getRegistry(const Ice::Current&) const;
    virtual RegistryPrx getLocalRegistry(const Ice::Current&) const;
    virtual QueryPrx getLocalQuery(const Ice::Current&) const;
    
    const Ice::CommunicatorPtr& getCommunicator() const;
    const TraceLevelsPtr& getTraceLevels() const;

    void activate(const LocatorAdapterInfo&, const RequestPtr&);
    void cancelActivate(const std::string&, const RequestPtr&);

    void activateFinished(const std::string&, const Ice::ObjectPrx&);
    void activateException(const std::string&, const Ice::Exception&);

    bool addPendingResolve(const std::string&, const Ice::AMD_Locator_findAdapterByIdPtr&);
    void removePendingResolve(const std::string&, int);

protected:

    const Ice::CommunicatorPtr _communicator;
    const DatabasePtr _database;
    const Ice::LocatorRegistryPrx _locatorRegistry;
    const RegistryPrx _localRegistry;
    const QueryPrx _localQuery;

    typedef std::set<RequestPtr> PendingRequests;
    typedef std::map<std::string, PendingRequests> PendingRequestsMap;

    PendingRequestsMap _pendingRequests;

    std::map<std::string, std::deque<Ice::AMD_Locator_findAdapterByIdPtr> > _resolves;
};

}

#endif
