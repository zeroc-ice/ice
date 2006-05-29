// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_LOCATOR_I_H
#define ICE_GRID_LOCATOR_I_H

#include <IceGrid/Internal.h>
#include <Ice/Locator.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;
    
class LocatorI;
typedef IceUtil::Handle<LocatorI> LocatorIPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class LocatorI : public Ice::Locator, public IceUtil::Mutex
{
    class Request : public IceUtil::Mutex, public IceUtil::Shared
    {
    public:

	Request(const Ice::AMD_Locator_findAdapterByIdPtr&, const LocatorIPtr&, const std::string&, bool,
		const std::vector<std::pair<std::string, AdapterPrx> >&, int, const TraceLevelsPtr&);

	void execute();
	void response(const Ice::ObjectPrx&);
	void exception(const Ice::Exception&); 

    private:

	void requestAdapter(const AdapterPrx&);
	void sendResponse();

	const Ice::AMD_Locator_findAdapterByIdPtr _amdCB;
	const LocatorIPtr _locator;
	const std::string _id;
	const bool _replicaGroup;
	const std::vector<std::pair<std::string, AdapterPrx> > _adapters;
	const TraceLevelsPtr _traceLevels;
	unsigned int _count;
	std::vector<std::pair<std::string, AdapterPrx> >::const_iterator _lastAdapter;
	std::vector<Ice::ObjectPrx> _proxies;
	std::auto_ptr<Ice::Exception> _exception;
    };
    typedef IceUtil::Handle<Request> RequestPtr;

public:

    LocatorI(const Ice::CommunicatorPtr&, const DatabasePtr&, const Ice::LocatorRegistryPrx&);

    virtual void findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr&, const Ice::Identity&, 
				      const Ice::Current&) const;

    virtual void findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr&, const ::std::string&, 
				       const Ice::Current&) const;

    virtual Ice::LocatorRegistryPrx getRegistry(const Ice::Current&) const;

    bool getDirectProxyRequest(const RequestPtr&, const AdapterPrx&);
    void getDirectProxyException(const AdapterPrx&, const std::string&, const Ice::Exception&);
    void getDirectProxyCallback(const Ice::Identity&, const Ice::ObjectPrx&);
    
    const Ice::CommunicatorPtr& getCommunicator() const;

protected:

    const Ice::CommunicatorPtr _communicator;
    const DatabasePtr _database;
    const Ice::LocatorRegistryPrx _locatorRegistry;

    typedef std::vector<RequestPtr> PendingRequests;
    typedef std::map<Ice::Identity, PendingRequests> PendingRequestsMap;

    PendingRequestsMap _pendingRequests;
};

}

#endif
