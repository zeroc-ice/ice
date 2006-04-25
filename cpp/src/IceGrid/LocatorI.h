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

class SessionI;
typedef IceUtil::Handle<SessionI> SessionIPtr;

class LocatorI : public Ice::Locator, public IceUtil::Mutex
{
    class Request : public IceUtil::Mutex, public IceUtil::Shared
    {
    public:

	Request(const Ice::AMD_Locator_findAdapterByIdPtr&, const LocatorIPtr&, const std::string&,
		const std::vector<std::pair<std::string, AdapterPrx> >&, int);

	void execute();
	void response(const Ice::ObjectPrx&);
	void exception(); 

    private:

	void requestAdapter(const AdapterPrx&);
	void sendResponse();

	const Ice::AMD_Locator_findAdapterByIdPtr _amdCB;
	const LocatorIPtr _locator;
	const std::string _id;
	const std::vector<std::pair<std::string, AdapterPrx> > _adapters;
	unsigned int _count;
	std::vector<std::pair<std::string, AdapterPrx> >::const_iterator _lastAdapter;
	std::vector<Ice::ObjectPrx> _proxies;
    };
    typedef IceUtil::Handle<Request> RequestPtr;

public:

    LocatorI(const Ice::CommunicatorPtr&, const DatabasePtr&, const Ice::LocatorRegistryPrx&, const SessionIPtr&);

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
    const SessionIPtr _session;

    typedef std::vector<RequestPtr> PendingRequests;
    typedef std::map<Ice::Identity, PendingRequests> PendingRequestsMap;

    PendingRequestsMap _pendingRequests;
};

}

#endif
