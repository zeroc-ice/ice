// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/LocatorRegistryI.h>
#include <IceGrid/ReplicaSessionManager.h>
#include <IceGrid/Database.h>
#include <IceGrid/Util.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

template<class AmdCB>
class SetDirectProxyCB : public AMI_Adapter_setDirectProxy
{
public:

    SetDirectProxyCB(const AmdCB& cb, 
                     const TraceLevelsPtr& traceLevels, 
                     const string& id, 
                     const Ice::ObjectPrx& proxy) : 
        _cb(cb), _traceLevels(traceLevels), _id(id), _proxy(proxy)                                                  
    {
    }

    virtual void ice_response()
    {
        if(_traceLevels->locator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "registered adapter `" << _id << "' endpoints: `";
            out << (_proxy ? _proxy->ice_toString() : string("")) << "'";
        }
        _cb->ice_response();
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        if(_traceLevels->locator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "failed to register adapter `" << _id << "' endpoints:\n" << ex;
        }
        
        try
        {
            ex.ice_throw();
        }
        catch(const AdapterActiveException&)
        {
            _cb->ice_exception(Ice::AdapterAlreadyActiveException());
            return;
        }
        catch(const Ice::ObjectNotExistException&)
        {
            _cb->ice_exception(Ice::AdapterNotFoundException()); // Expected if the adapter was destroyed.
            return;
        }
        catch(const Ice::LocalException&)
        {
            _cb->ice_exception(Ice::AdapterNotFoundException());
            return;
        }

        assert(false);
    }

private:

    const AmdCB _cb;
    const TraceLevelsPtr _traceLevels;
    const string _id;
    const Ice::ObjectPrx _proxy;
};

template<class AmdCB> SetDirectProxyCB<AmdCB>*
newSetDirectProxyCB(const AmdCB& cb, const TraceLevelsPtr& traceLevels, const string& id, const Ice::ObjectPrx& p)
{
    return new SetDirectProxyCB<AmdCB>(cb, traceLevels, id, p);
}

class AMI_Server_setProcessI : public AMI_Server_setProcess
{
public:

    AMI_Server_setProcessI(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
                           const TraceLevelsPtr& traceLevels,
                           const string& id,
                           const Ice::ObjectPrx& proxy) : 
        _cb(cb), _traceLevels(traceLevels), _id(id), _proxy(proxy)
    {
    }

    virtual void ice_response()
    {
        if(_traceLevels->locator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "registered server `" << _id << "' process proxy: `";
            out << (_proxy ? _proxy->ice_toString() : string("")) << "'";
        }
        _cb->ice_response();
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        if(_traceLevels->locator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "failed to register server process proxy `" << _id << "':\n" << ex;
        }

        try
        {
            ex.ice_throw();
        }
        catch(const Ice::ObjectNotExistException&)
        {
            // Expected if the server was destroyed.
            _cb->ice_exception(Ice::ServerNotFoundException()); 
            return;
        }
        catch(const Ice::LocalException&)
        {
            _cb->ice_exception(Ice::ServerNotFoundException());
            return;
        }

        assert(false);
    }

private:

    const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr _cb;
    const TraceLevelsPtr _traceLevels;
    const string _id;
    const Ice::ObjectPrx _proxy;
};

};

LocatorRegistryI::LocatorRegistryI(const DatabasePtr& database,
                                   bool dynamicRegistration, 
                                   bool master,
                                   ReplicaSessionManager& session) :
    _database(database),
    _dynamicRegistration(dynamicRegistration),
    _master(master),
    _session(session)
{
}

void 
LocatorRegistryI::setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb,
                                              const string& adapterId, 
                                              const Ice::ObjectPrx& proxy,
                                              const Ice::Current&)
{
    setAdapterDirectProxy(newSetDirectProxyCB(cb, _database->getTraceLevels(), adapterId, proxy),
                          adapterId, 
                          "",
                          proxy);
}

void 
LocatorRegistryI::setReplicatedAdapterDirectProxy_async(
    const Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr& cb,
    const string& adapterId, 
    const string& replicaGroupId,
    const Ice::ObjectPrx& proxy,
    const Ice::Current&)
{
    setAdapterDirectProxy(newSetDirectProxyCB(cb, _database->getTraceLevels(), adapterId, proxy),
                          adapterId, 
                          replicaGroupId,
                          proxy);
}

void
LocatorRegistryI::setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
                                              const string& id,
                                              const Ice::ProcessPrx& proxy, 
                                              const Ice::Current&)
{
    try
    {
        //
        // Get the server from the registry and set its process proxy.
        //
        // NOTE: We pass false to the getServer call to indicate that
        // we don't necessarily want an up-to-date adapter proxy. This
        // is needed for the session activation mode for cases where
        // the server is released during the server startup.
        //
        AMI_Server_setProcessPtr amiCB = new AMI_Server_setProcessI(cb, _database->getTraceLevels(), id, proxy);
        _database->getServer(id)->getProxy(false)->setProcess_async(amiCB, proxy);
    }
    catch(const ServerNotExistException&)
    {
        cb->ice_exception(Ice::ServerNotFoundException());
    }
    catch(const Ice::Exception& ex)
    {
        const TraceLevelsPtr traceLevels = _database->getTraceLevels();
        if(traceLevels->locator > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
            out << "couldn't register server `" << id << "' process proxy:\n" << toString(ex);
        }
        cb->ice_exception(Ice::ServerNotFoundException());
    }
}

void
LocatorRegistryI::setAdapterDirectProxy(const AMI_Adapter_setDirectProxyPtr& amiCB,
                                        const string& adapterId, 
                                        const string& replicaGroupId,
                                        const Ice::ObjectPrx& proxy)
{
    //
    // Ignore request with empty adapter id.
    //
    if(adapterId.empty())
    {
        amiCB->ice_response();
        return;
    }

    int nRetry = 5;
    do
    {
        try
        {
            //
            // Get the adapter from the registry and set its direct proxy.
            //
            ServerAdapterEntryPtr adapter = ServerAdapterEntryPtr::dynamicCast(_database->getAdapter(adapterId));
            if(!adapter)
            {
                throw Ice::AdapterNotFoundException();
            }
            adapter->getProxy(replicaGroupId, false)->setDirectProxy_async(amiCB, proxy);
            return;
        }
        catch(const AdapterNotExistException&)
        {
            if(!_dynamicRegistration)
            {
                throw Ice::AdapterNotFoundException();
            }
        }
        catch(const Ice::Exception& ex)
        {
            const TraceLevelsPtr traceLevels = _database->getTraceLevels();
            if(traceLevels->locator > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                out << "couldn't register adapter `" << adapterId << "' endpoints:\n" << toString(ex);
            }       
            throw Ice::AdapterNotFoundException();
        }
        
        assert(_dynamicRegistration);
        if(_master)
        {
            try
            {
                _database->setAdapterDirectProxy(adapterId, replicaGroupId, proxy);
                amiCB->ice_response();
                return;
            }
            catch(const AdapterExistsException&)
            {
                // Continue
            }
        }
        else
        {
            ReplicaSessionPrx session = _session.getSession();
            if(!session)
            {
                const TraceLevelsPtr traceLevels = _database->getTraceLevels();
                if(traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't register adapter `" << adapterId << "' endpoints with master:\n";
                    out << "no session established with the master";
                }
                throw Ice::AdapterNotFoundException();
            }

            try
            {
                session->setAdapterDirectProxy(adapterId, replicaGroupId, proxy);
                amiCB->ice_response();
                return;
            }
            catch(const AdapterExistsException&)
            {
                // Continue
            }
            catch(const AdapterNotExistException&)
            {
                throw Ice::AdapterNotFoundException(); // Dynamic registration not allowed on the master.
            }
            catch(const Ice::LocalException& ex)
            {
                const TraceLevelsPtr traceLevels = _database->getTraceLevels();
                if(traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't register adapter `" << adapterId << "' endpoints with master:\n" << toString(ex);
                }
                throw Ice::AdapterNotFoundException();
            }
        }
    }
    while(nRetry-- > 0);
    throw Ice::AdapterNotFoundException();
}
