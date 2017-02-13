// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/UUID.h>
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

template<class T>
class AllocateObject : public ObjectAllocationRequest
{
    typedef IceUtil::Handle<T> TPtr;

public:

    AllocateObject(const SessionIPtr& session, const TPtr& cb) :
        ObjectAllocationRequest(session), _cb(cb)
    {
    }

    virtual void
    response(const Ice::ObjectPrx& proxy)
    {
        assert(_cb);
        _cb->ice_response(proxy);
        _cb = 0;
    }

    virtual void
    exception(const Ice::UserException& ex)
    {
        assert(_cb);
        _cb->ice_exception(ex);
        _cb = 0;
    }

private:

    TPtr _cb;
};

template<class T> AllocateObject<T>*
newAllocateObject(const SessionIPtr& session, const IceUtil::Handle<T>& cb)
{
    return new AllocateObject<T>(session, cb);
}

}

BaseSessionI::BaseSessionI(const string& id, const string& prefix, const DatabasePtr& database) :
    _id(id),
    _prefix(prefix),
    _traceLevels(database->getTraceLevels()),
    _database(database),
    _destroyed(false),
    _timestamp(IceUtil::Time::now(IceUtil::Time::Monotonic))
{
    if(_traceLevels && _traceLevels->session > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _prefix << " session `" << _id << "' created";
    }
}

BaseSessionI::~BaseSessionI()
{
}

void
BaseSessionI::keepAlive(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
        Ice::ObjectNotExistException ex(__FILE__, __LINE__);
        ex.id = current.id;
        throw ex;
    }

    _timestamp = IceUtil::Time::now(IceUtil::Time::Monotonic);

    if(_traceLevels->session > 1)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _prefix << " session `" << _id << "' keep alive";
    }
}

void
BaseSessionI::destroyImpl(bool /*shutdown*/)
{
    Lock sync(*this);
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

IceUtil::Time
BaseSessionI::timestamp() const
{
    Lock sync(*this);
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

Glacier2::IdentitySetPrx
BaseSessionI::getGlacier2IdentitySet()
{
    assert(_servantManager);
    return _servantManager->getGlacier2IdentitySet(this);
}

Glacier2::StringSetPrx
BaseSessionI::getGlacier2AdapterIdSet()
{
    assert(_servantManager);
    return _servantManager->getGlacier2AdapterIdSet(this);
}

SessionI::SessionI(const string& id, const DatabasePtr& database, const IceUtil::TimerPtr& timer) :
    BaseSessionI(id, "client", database),
    _timer(timer),
    _allocationTimeout(-1)
{
}

SessionI::~SessionI()
{
}

Ice::ObjectPrx
SessionI::_register(const SessionServantManagerPtr& servantManager, const Ice::ConnectionPtr& con)
{
    //
    // This is supposed to be called after creation only, no need to synchronize.
    //
    _servantManager = servantManager;
    return _servantManager->addSession(this, con, "");
}

void
SessionI::allocateObjectById_async(const AMD_Session_allocateObjectByIdPtr& cb,
                                   const Ice::Identity& id,
                                   const Ice::Current&)
{
    _database->getAllocatableObject(id)->allocate(newAllocateObject(this, cb));
}

void
SessionI::allocateObjectByType_async(const AMD_Session_allocateObjectByTypePtr& cb,
                                     const string& type,
                                     const Ice::Current&)
{
    _database->getAllocatableObjectCache().allocateByType(type, newAllocateObject(this, cb));
}

void
SessionI::releaseObject(const Ice::Identity& id, const Ice::Current&)
{
    _database->getAllocatableObject(id)->release(this);
}

void
SessionI::setAllocationTimeout(int timeout, const Ice::Current&)
{
    Lock sync(*this);
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
    Lock sync(*this);
    return _allocationTimeout;
}

bool
SessionI::addAllocationRequest(const AllocationRequestPtr& request)
{
    Lock sync(*this);
    if(_destroyed)
    {
        return false;
    }
    _requests.insert(request);
    return true;
}

void
SessionI::removeAllocationRequest(const AllocationRequestPtr& request)
{
    Lock sync(*this);
    if(_destroyed)
    {
        return;
    }
    _requests.erase(request);
}

void
SessionI::addAllocation(const AllocatablePtr& allocatable)
{
    Lock sync(*this);
    if(_destroyed)
    {
        throw SessionDestroyedException();
    }
    _allocations.insert(allocatable);
}

void
SessionI::removeAllocation(const AllocatablePtr& allocatable)
{
    Lock sync(*this);
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

    _servantManager->removeSession(this);

    //
    // NOTE: The _requests and _allocations attributes are immutable
    // once the session is destroyed so we don't need mutex protection
    // here to access them.
    //

    for(set<AllocationRequestPtr>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        (*p)->cancel(AllocationException("session destroyed"));
    }
    _requests.clear();

    for(set<AllocatablePtr>::const_iterator q = _allocations.begin(); q != _allocations.end(); ++q)
    {
        try
        {
            (*q)->release(this);
        }
        catch(const AllocationException&)
        {
        }
    }
    _allocations.clear();
}

ClientSessionFactory::ClientSessionFactory(const SessionServantManagerPtr& servantManager,
                                           const DatabasePtr& database,
                                           const IceUtil::TimerPtr& timer,
                                           const ReapThreadPtr& reaper) :
    _servantManager(servantManager),
    _database(database),
    _timer(timer),
    _reaper(reaper),
    _filters(false)
{
    if(_servantManager) // Not set if Glacier2 session manager adapter not enabled
    {
        Ice::PropertiesPtr properties = _database->getCommunicator()->getProperties();
        const_cast<bool&>(_filters) = properties->getPropertyAsIntWithDefault("IceGrid.Registry.SessionFilters", 0) > 0;
    }
}

Glacier2::SessionPrx
ClientSessionFactory::createGlacier2Session(const string& sessionId, const Glacier2::SessionControlPrx& ctl)
{
    assert(_servantManager);

    SessionIPtr session = createSessionServant(sessionId, ctl);
    Ice::ObjectPrx proxy = session->_register(_servantManager, 0);

    int timeout = 0;
    if(ctl)
    {
        try
        {
            if(_filters)
            {
                Ice::IdentitySeq ids;
                Ice::Identity queryId;
                queryId.category = _database->getInstanceName();
                queryId.name = "Query";
                ids.push_back(queryId);

                _servantManager->setSessionControl(session, ctl, ids);
            }
            timeout = ctl->getSessionTimeout();
        }
        catch(const Ice::LocalException& e)
        {
            session->destroy(Ice::Current());

            Ice::Warning out(_database->getTraceLevels()->logger);
            out << "Failed to callback Glacier2 session control object:\n" << e;

            Glacier2::CannotCreateSessionException ex;
            ex.reason = "internal server error";
            throw ex;
        }
    }

    _reaper->add(new SessionReapable<SessionI>(_database->getTraceLevels()->logger, session), timeout);
    return Glacier2::SessionPrx::uncheckedCast(proxy);
}

SessionIPtr
ClientSessionFactory::createSessionServant(const string& userId, const Glacier2::SessionControlPrx&)
{
    return new SessionI(userId, _database, _timer);
}

const TraceLevelsPtr&
ClientSessionFactory::getTraceLevels() const
{
    return _database->getTraceLevels();
}

ClientSessionManagerI::ClientSessionManagerI(const ClientSessionFactoryPtr& factory) : _factory(factory)
{
}

Glacier2::SessionPrx
ClientSessionManagerI::create(const string& user, const Glacier2::SessionControlPrx& ctl, const Ice::Current&)
{
    return _factory->createGlacier2Session(user, ctl);
}

ClientSSLSessionManagerI::ClientSSLSessionManagerI(const ClientSessionFactoryPtr& factory) : _factory(factory)
{
}

Glacier2::SessionPrx
ClientSSLSessionManagerI::create(const Glacier2::SSLInfo& info,
                                 const Glacier2::SessionControlPrx& ctl,
                                 const Ice::Current&)
{
    string userDN;
    if(!info.certs.empty()) // TODO: Require userDN?
    {
        try
        {
            IceSSL::CertificatePtr cert = IceSSL::Certificate::decode(info.certs[0]);
            userDN = cert->getSubjectDN();
        }
        catch(const Ice::Exception& e)
        {
            // This shouldn't happen, the SSLInfo is supposed to be encoded by Glacier2.
            Ice::Error out(_factory->getTraceLevels()->logger);
            out << "SSL session manager couldn't decode SSL certificates:\n" << e;

            Glacier2::CannotCreateSessionException ex;
            ex.reason = "internal server error";
            throw ex;
        }
    }

    return _factory->createGlacier2Session(userDN, ctl);
}

