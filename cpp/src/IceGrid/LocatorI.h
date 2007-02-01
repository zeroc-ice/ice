// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_LOCATOR_I_H
#define ICE_GRID_LOCATOR_I_H

#include <IceGrid/Internal.h>
#include <IceGrid/Locator.h>

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
    class Request : public IceUtil::Mutex, public IceUtil::Shared
    {
    public:

        Request(const Ice::AMD_Locator_findAdapterByIdPtr&, const LocatorIPtr&, const std::string&, bool,
                const LocatorAdapterInfoSeq&, int, const TraceLevelsPtr&);

        void execute();
        void response(const Ice::ObjectPrx&);
        void exception(const Ice::Exception&); 

    private:

        void requestAdapter(const LocatorAdapterInfo&);
        void sendResponse();

        const Ice::AMD_Locator_findAdapterByIdPtr _amdCB;
        const LocatorIPtr _locator;
        const std::string _id;
        const bool _replicaGroup;
        LocatorAdapterInfoSeq _adapters;
        const TraceLevelsPtr _traceLevels;
        unsigned int _count;
        LocatorAdapterInfoSeq::const_iterator _lastAdapter;
        std::vector<Ice::ObjectPrx> _proxies;
        std::auto_ptr<Ice::Exception> _exception;
    };
    typedef IceUtil::Handle<Request> RequestPtr;

public:

    LocatorI(const Ice::CommunicatorPtr&, const DatabasePtr&, const Ice::LocatorRegistryPrx&, const RegistryPrx&,
             const QueryPrx&);

    virtual void findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr&, const Ice::Identity&, 
                                      const Ice::Current&) const;

    virtual void findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr&, const ::std::string&, 
                                       const Ice::Current&) const;

    virtual Ice::LocatorRegistryPrx getRegistry(const Ice::Current&) const;
    virtual RegistryPrx getLocalRegistry(const Ice::Current&) const;
    virtual QueryPrx getLocalQuery(const Ice::Current&) const;

    bool getDirectProxyRequest(const RequestPtr&, const LocatorAdapterInfo&);
    void getDirectProxyException(const LocatorAdapterInfo&, const std::string&, const Ice::Exception&);
    void getDirectProxyCallback(const Ice::Identity&, const Ice::ObjectPrx&);
    
    const Ice::CommunicatorPtr& getCommunicator() const;

protected:

    const Ice::CommunicatorPtr _communicator;
    const DatabasePtr _database;
    const Ice::LocatorRegistryPrx _locatorRegistry;
    const RegistryPrx _localRegistry;
    const QueryPrx _localQuery;

    typedef std::vector<RequestPtr> PendingRequests;
    typedef std::map<Ice::Identity, PendingRequests> PendingRequestsMap;

    PendingRequestsMap _pendingRequests;
};

}

#endif
