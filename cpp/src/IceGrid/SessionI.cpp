//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Ice/UUID.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/QueryI.h>
#include <IceGrid/LocatorI.h>
#include <IceGrid/Database.h>
#include <IceGrid/Admin.h>

#include <IceSSL/Plugin.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

class AllocateObject final : public ObjectAllocationRequest
{

public:

    AllocateObject(const shared_ptr<SessionI>& session,
                   function<void(const shared_ptr<Ice::ObjectPrx>& returnValue)>&& response,
                   function<void(exception_ptr)>&& exception) :
        ObjectAllocationRequest(session), _response(move(response)), _exception(move(exception))
    {
    }

    void
    response(const shared_ptr<Ice::ObjectPrx>& proxy) override
    {
        assert(_response);
        _response(proxy);
        _response = nullptr;

    }

    void
    exception(exception_ptr ex) override
    {
        assert(_exception);
        _exception(ex);
        _exception = nullptr;
    }

private:

    function<void(const shared_ptr<Ice::ObjectPrx>& returnValue)> _response;
    function<void(exception_ptr)> _exception;
};

}

BaseSessionI::BaseSessionI(const string& id, const string& prefix, const shared_ptr<Database>& database) :
    _id(id),
    _prefix(prefix),
    _traceLevels(database->getTraceLevels()),
    _database(database),
    _destroyed(false),
    _timestamp(chrono::steady_clock::now())
{
    if(_traceLevels && _traceLevels->session > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _prefix << " session `" << _id << "' created";
    }
}

void
BaseSessionI::keepAlive(const Ice::Current& current)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
    }

    _timestamp = chrono::steady_clock::now();

    if(_traceLevels->session > 1)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _prefix << " session `" << _id << "' keep alive";
    }
}

void
BaseSessionI::destroyImpl(bool)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    _destroyed = true;

    if(_traceLevels && _traceLevels->session > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _prefix << " session `" << _id << "' destroyed";
    }
}

std::chrono::steady_clock::time_point
BaseSessionI::timestamp() const
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    return _timestamp;
}

void
BaseSessionI::shutdown()
{
    destroyImpl(true);
}

shared_ptr<Glacier2::IdentitySetPrx>
BaseSessionI::getGlacier2IdentitySet()
{
    assert(_servantManager);
    return _servantManager->getGlacier2IdentitySet(shared_from_this());
}

shared_ptr<Glacier2::StringSetPrx>
BaseSessionI::getGlacier2AdapterIdSet()
{
    assert(_servantManager);
    return _servantManager->getGlacier2AdapterIdSet(shared_from_this());
}

SessionI::SessionI(const string& id, const shared_ptr<Database>& database,
                   const IceUtil::TimerPtr& timer) :
    BaseSessionI(id, "client", database),
    _timer(timer),
    _allocationTimeout(-1)
{
}

shared_ptr<Ice::ObjectPrx>
SessionI::_register(const shared_ptr<SessionServantManager>& servantManager, const shared_ptr<Ice::Connection>& con)
{
    //
    // This is supposed to be called after creation only, no need to synchronize.
    //
    _servantManager = servantManager;
    return _servantManager->addSession(shared_from_this(), con, "");
}

void
SessionI::allocateObjectByIdAsync(Ice::Identity id,
                                 function<void(const shared_ptr<Ice::ObjectPrx>& returnValue)> response,
                                 function<void(exception_ptr)> exception,
                                 const Ice::Current&)
{
    auto allocatedObject = make_shared<AllocateObject>(static_pointer_cast<SessionI>(shared_from_this()),
                                                       move(response), move(exception));
    _database->getAllocatableObject(id)->allocate(move(allocatedObject));
}

void
SessionI::allocateObjectByTypeAsync(string type,
                                    function<void(const shared_ptr<Ice::ObjectPrx>& returnValue)> response,
                                    function<void(exception_ptr)> exception,
                                    const Ice::Current&)
{
    auto allocatedObject = make_shared<AllocateObject>(static_pointer_cast<SessionI>(shared_from_this()),
                                                       move(response), move(exception));
    _database->getAllocatableObjectCache().allocateByType(type, move(allocatedObject));
}

void
SessionI::releaseObject(Ice::Identity id, const Ice::Current&)
{
    _database->getAllocatableObject(id)->release(static_pointer_cast<SessionI>(shared_from_this()));
}

void
SessionI::setAllocationTimeout(int timeout, const Ice::Current&)
{
    lock_guard lock(_mutex);
    _allocationTimeout = timeout;
}

void
SessionI::destroy(const Ice::Current&)
{
    destroyImpl(false);
}

int
SessionI::getAllocationTimeout() const
{
    lock_guard lock(_mutex);
    return _allocationTimeout;
}

bool
SessionI::addAllocationRequest(const shared_ptr<AllocationRequest>& request)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        return false;
    }
    _requests.insert(request);
    return true;
}

void
SessionI::removeAllocationRequest(const shared_ptr<AllocationRequest>& request)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        return;
    }
    _requests.erase(request);
}

void
SessionI::addAllocation(const shared_ptr<Allocatable>& allocatable)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        throw SessionDestroyedException();
    }
    _allocations.insert(allocatable);
}

void
SessionI::removeAllocation(const shared_ptr<Allocatable>& allocatable)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        return;
    }
    _allocations.erase(allocatable);
}

void
SessionI::destroyImpl(bool shutdown)
{
    BaseSessionI::destroyImpl(shutdown);

    _servantManager->removeSession(shared_from_this());

    //
    // NOTE: The _requests and _allocations attributes are immutable
    // once the session is destroyed so we don't need mutex protection
    // here to access them.
    //
    for(const auto& request : _requests)
    {
        request->cancel(make_exception_ptr(AllocationException("session destroyed")));
    }
    _requests.clear();

    for(const auto& allocation : _allocations)
    {
        try
        {
            allocation->release(dynamic_pointer_cast<SessionI>(shared_from_this()));
        }
        catch(const AllocationException&)
        {
        }
    }
    _allocations.clear();
}

ClientSessionFactory::ClientSessionFactory(const shared_ptr<SessionServantManager>& servantManager,
                                           const shared_ptr<Database>& database,
                                           const IceUtil::TimerPtr& timer,
                                           const shared_ptr<ReapThread>& reaper) :
    _servantManager(servantManager),
    _database(database),
    _timer(timer),
    _reaper(reaper),
    _filters(false)
{
    if(_servantManager) // Not set if Glacier2 session manager adapter not enabled
    {
        auto properties = _database->getCommunicator()->getProperties();
        const_cast<bool&>(_filters) = properties->getPropertyAsIntWithDefault("IceGrid.Registry.SessionFilters", 0) > 0;
    }
}

shared_ptr<Glacier2::SessionPrx>
ClientSessionFactory::createGlacier2Session(const string& sessionId,
                                            const shared_ptr<Glacier2::SessionControlPrx>& ctl)
{
    assert(_servantManager);

    auto session = createSessionServant(sessionId, ctl);
    auto proxy = session->_register(_servantManager, 0);

    chrono::seconds timeout = 0s;
    if(ctl)
    {
        try
        {
            if(_filters)
            {
                Ice::Identity queryId = { "Query", _database->getInstanceName() };
                _servantManager->setSessionControl(session, ctl, { move(queryId) });
            }
            timeout = chrono::seconds(ctl->getSessionTimeout());
        }
        catch(const Ice::LocalException& e)
        {
            session->destroy(Ice::Current());

            Ice::Warning out(_database->getTraceLevels()->logger);
            out << "Failed to callback Glacier2 session control object:\n" << e;

            throw Glacier2::CannotCreateSessionException("internal server error");
        }
    }

    _reaper->add(make_shared<SessionReapable<SessionI>>(_database->getTraceLevels()->logger, session), timeout);
    return Ice::uncheckedCast<Glacier2::SessionPrx>(proxy);
}

shared_ptr<SessionI>
ClientSessionFactory::createSessionServant(const string& userId, const shared_ptr<Glacier2::SessionControlPrx>&)
{
    return make_shared<SessionI>(userId, _database, _timer);
}

const shared_ptr<TraceLevels>&
ClientSessionFactory::getTraceLevels() const
{
    return _database->getTraceLevels();
}

ClientSessionManagerI::ClientSessionManagerI(const shared_ptr<ClientSessionFactory>& factory) : _factory(factory)
{
}

shared_ptr<Glacier2::SessionPrx>
ClientSessionManagerI::create(string user, shared_ptr<Glacier2::SessionControlPrx> ctl, const Ice::Current&)
{
    return _factory->createGlacier2Session(move(user), move(ctl));
}

ClientSSLSessionManagerI::ClientSSLSessionManagerI(const shared_ptr<ClientSessionFactory>& factory) : _factory(factory)
{
}

shared_ptr<Glacier2::SessionPrx>
ClientSSLSessionManagerI::create(Glacier2::SSLInfo info,
                                 shared_ptr<Glacier2::SessionControlPrx> ctl,
                                 const Ice::Current&)
{
    string userDN;
    if(!info.certs.empty()) // TODO: Require userDN?
    {
        try
        {
            auto cert = IceSSL::Certificate::decode(info.certs[0]);
            userDN = cert->getSubjectDN();
        }
        catch(const Ice::Exception& e)
        {
            // This shouldn't happen, the SSLInfo is supposed to be encoded by Glacier2.
            Ice::Error out(_factory->getTraceLevels()->logger);
            out << "SSL session manager couldn't decode SSL certificates:\n" << e;

            throw Glacier2::CannotCreateSessionException("internal server error");
        }
    }

    return _factory->createGlacier2Session(userDN, ctl);
}
