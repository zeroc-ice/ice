//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceGrid/RegistryAdminRouter.h>
#include <IceGrid/SynchronizationException.h>
#include <Ice/Ice.h>

using namespace IceGrid;
using namespace Ice;
using namespace std;

namespace
{

class SynchronizationCallbackI final : public SynchronizationCallback
{
public:

    SynchronizationCallbackI(const shared_ptr<RegistryServerAdminRouter>& adminRouter,
                             const pair<const Byte*, const Byte*>& inParams,
                             function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                             function<void(exception_ptr)> exception,
                             const Current& current) :
        _adminRouter(adminRouter),
        _response(move(response)),
        _exception(move(exception)),
        _inParams(inParams.first, inParams.second),
        _current(current)
    {
    }

    void synchronized()
    {
        //
        // Retry to forward the call.
        //
        _adminRouter->ice_invokeAsync({ &_inParams[0], &_inParams[0] + _inParams.size() },
                                      move(_response), move(_exception), _current);
    }

    void synchronized(exception_ptr)
    {
        _exception(make_exception_ptr(Ice::ObjectNotExistException(__FILE__, __LINE__)));
    }

private:

    const shared_ptr<RegistryServerAdminRouter> _adminRouter;
    function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> _response;
    function<void(exception_ptr)> _exception;
    const vector<Byte> _inParams;
    const Current _current;
};

}

RegistryServerAdminRouter::RegistryServerAdminRouter(const shared_ptr<Database>& database) :
    AdminRouter(database->getTraceLevels()),
    _database(database)
{
}

void
RegistryServerAdminRouter::ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*> inParams,
                                       function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                                       function<void(exception_ptr)> exception,
                                       const Ice::Current& current)
{
    shared_ptr<ObjectPrx> target;

    try
    {
        auto server = _database->getServer(current.id.name);
        try
        {
            target = server->getAdminProxy();
        }
        catch(const SynchronizationException&)
        {
            server->addSyncCallback(make_shared<SynchronizationCallbackI>(shared_from_this(), inParams,
                                                                          move(response), move(exception), current));
            return; // Wait for the server synchronization to complete and retry.
        }
    }
    catch(const ServerNotExistException&)
    {
    }
    catch(const NodeUnreachableException&)
    {
    }
    catch(const DeploymentException&)
    {
    }

    if(target == nullptr)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }

    target = target->ice_facet(current.facet);

    invokeOnTarget(target, inParams, move(response), move(exception), current);
}

RegistryNodeAdminRouter::RegistryNodeAdminRouter(const string& collocNodeName, const shared_ptr<Database>& database) :
    AdminRouter(database->getTraceLevels()),
    _collocNodeName(collocNodeName),
    _database(database)
{
}

void
RegistryNodeAdminRouter::ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*> inParams,
                                       function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                                       function<void(exception_ptr)> exception,
                                       const Ice::Current& current)
{
    shared_ptr<ObjectPrx> target;

    if(!_collocNodeName.empty() && current.id.name == _collocNodeName)
    {
        // Straight to the local Admin object
        target = current.adapter->getCommunicator()->getAdmin();
    }
    else
    {
        try
        {
            target = _database->getNode(current.id.name)->getAdminProxy();
        }
        catch(const NodeUnreachableException&)
        {
        }
        catch(const NodeNotExistException&)
        {
        }

        if(target == 0)
        {
            if(_traceLevels->admin > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
                out << "could not find Admin proxy for node `" << current.id.name << "'";
            }

            throw ObjectNotExistException(__FILE__, __LINE__);
        }
    }

    target = target->ice_facet(current.facet);

    invokeOnTarget(target, inParams, move(response), move(exception), current);
}

RegistryReplicaAdminRouter::RegistryReplicaAdminRouter(const string& name,
                                                       const shared_ptr<Database>& database) :
    AdminRouter(database->getTraceLevels()),
    _name(name),
    _database(database)
{
}

void
RegistryReplicaAdminRouter::ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*> inParams,
                                       function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                                       function<void(exception_ptr)> exception,
                                       const Ice::Current& current)
{
    shared_ptr<ObjectPrx> target;

    if(current.id.name == _name)
    {
        // Straight to the local Admin object
        target = current.adapter->getCommunicator()->getAdmin();
    }
    else
    {
        try
        {
            // Forward to Admin object in remote replica
            target = _database->getReplica(current.id.name)->getAdminProxy();
        }
        catch(const RegistryNotExistException&)
        {
        }
    }

    if(target == nullptr)
    {
        if(_traceLevels->admin > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
            out << "could not find Admin proxy for replica `" << current.id.name << "'";
        }

        throw ObjectNotExistException(__FILE__, __LINE__);
    }

    target = target->ice_facet(current.facet);

    invokeOnTarget(target, inParams, move(response), move(exception), current);
}
