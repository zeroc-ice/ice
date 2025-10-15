// Copyright (c) ZeroC, Inc.

#include "SessionI.h"
#include "../Ice/SSL/SSLUtil.h"
#include "Database.h"
#include "Ice/Ice.h"
#include "IceGrid/Admin.h"
#include "LocatorI.h"
#include "QueryI.h"

using namespace std;
using namespace IceGrid;

namespace IceGrid
{
    class AllocateObject final : public ObjectAllocationRequest
    {
    public:
        AllocateObject(
            const shared_ptr<SessionI>& session,
            function<void(const Ice::ObjectPrx&)>&& response,
            function<void(exception_ptr)>&& exception)
            : ObjectAllocationRequest(session),
              _response(std::move(response)),
              _exception(std::move(exception))
        {
        }

        void response(const Ice::ObjectPrx& proxy) final
        {
            assert(_response);
            _response(proxy);
            _response = nullptr;
        }

        void exception(exception_ptr ex) final
        {
            assert(_exception);
            _exception(ex);
            _exception = nullptr;
        }

    private:
        function<void(const Ice::ObjectPrx& returnValue)> _response;
        function<void(exception_ptr)> _exception;
    };
}

BaseSessionI::BaseSessionI(string id, string prefix, const shared_ptr<Database>& database)
    : _id(std::move(id)),
      _prefix(std::move(prefix)),
      _traceLevels(database->getTraceLevels()),
      _database(database)
{
    if (_traceLevels && _traceLevels->session > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _prefix << " session '" << _id << "' created";
    }
}

void
BaseSessionI::destroyImpl(bool)
{
    lock_guard lock(_mutex);
    if (_destroyed)
    {
        throw Ice::ObjectNotExistException{__FILE__, __LINE__};
    }
    _destroyed = true;

    if (_traceLevels && _traceLevels->session > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
        out << _prefix << " session '" << _id << "' destroyed";
    }
}

optional<chrono::steady_clock::time_point>
BaseSessionI::timestamp() const noexcept
{
    lock_guard lock(_mutex);
    if (_destroyed)
    {
        return nullopt;
    }
    return std::chrono::steady_clock::time_point::min(); // not used
}

void
BaseSessionI::shutdown()
{
    destroyImpl(true);
}

optional<Glacier2::IdentitySetPrx>
BaseSessionI::getGlacier2IdentitySet()
{
    assert(_servantManager);
    return _servantManager->getGlacier2IdentitySet(shared_from_this());
}

optional<Glacier2::StringSetPrx>
BaseSessionI::getGlacier2AdapterIdSet()
{
    assert(_servantManager);
    return _servantManager->getGlacier2AdapterIdSet(shared_from_this());
}

SessionI::SessionI(const string& id, const shared_ptr<Database>& database, IceInternal::TimerPtr timer)
    : BaseSessionI(id, "client", database),
      _timer(std::move(timer))
{
}

Ice::ObjectPrx
SessionI::_register(const shared_ptr<SessionServantManager>& servantManager, const shared_ptr<Ice::Connection>& con)
{
    //
    // This is supposed to be called after creation only, no need to synchronize.
    //
    _servantManager = servantManager;
    return _servantManager->addSession(shared_from_this(), con, "");
}

void
SessionI::allocateObjectByIdAsync(
    Ice::Identity id,
    function<void(const optional<Ice::ObjectPrx>& returnValue)> response,
    function<void(exception_ptr)> exception,
    const Ice::Current&)
{
    auto allocatedObject = make_shared<AllocateObject>(
        static_pointer_cast<SessionI>(shared_from_this()),
        std::move(response),
        std::move(exception));
    _database->getAllocatableObject(id)->allocate(std::move(allocatedObject));
}

void
SessionI::allocateObjectByTypeAsync(
    string type,
    function<void(const optional<Ice::ObjectPrx>& returnValue)> response,
    function<void(exception_ptr)> exception,
    const Ice::Current&)
{
    auto allocatedObject = make_shared<AllocateObject>(
        static_pointer_cast<SessionI>(shared_from_this()),
        std::move(response),
        std::move(exception));
    _database->getAllocatableObjectCache().allocateByType(type, std::move(allocatedObject));
}

void
SessionI::releaseObjectAsync(
    Ice::Identity id,
    function<void()> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    _database->getAllocatableObject(id)->release(static_pointer_cast<SessionI>(shared_from_this()));
    response();
}

void
SessionI::setAllocationTimeoutAsync(
    int32_t timeout,
    function<void()> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    {
        lock_guard lock(_mutex);
        _allocationTimeout = timeout;
    }
    response();
}

void
SessionI::destroyAsync(function<void()> response, function<void(std::exception_ptr)>, const Ice::Current&)
{
    destroy();
    response();
}

void
SessionI::destroy()
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
    if (_destroyed)
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
    if (_destroyed)
    {
        return;
    }
    _requests.erase(request);
}

void
SessionI::addAllocation(const shared_ptr<Allocatable>& allocatable)
{
    lock_guard lock(_mutex);
    if (_destroyed)
    {
        throw SessionDestroyedException();
    }
    _allocations.insert(allocatable);
}

void
SessionI::removeAllocation(const shared_ptr<Allocatable>& allocatable)
{
    lock_guard lock(_mutex);
    if (_destroyed)
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
    for (const auto& request : _requests)
    {
        request->cancel(make_exception_ptr(AllocationException("session destroyed")));
    }
    _requests.clear();

    for (const auto& allocation : _allocations)
    {
        try
        {
            allocation->release(dynamic_pointer_cast<SessionI>(shared_from_this()));
        }
        catch (const AllocationException&)
        {
        }
    }
    _allocations.clear();
}

ClientSessionFactory::ClientSessionFactory(
    const shared_ptr<SessionServantManager>& servantManager,
    const shared_ptr<Database>& database,
    IceInternal::TimerPtr timer,
    const shared_ptr<ReapThread>& reaper)
    : _servantManager(servantManager),
      _database(database),
      _timer(std::move(timer)),
      _reaper(reaper)
{
    if (_servantManager) // Not set if Glacier2 session manager adapter not enabled
    {
        auto properties = _database->getCommunicator()->getProperties();
        const_cast<bool&>(_filters) = properties->getIcePropertyAsInt("IceGrid.Registry.SessionFilters") > 0;
    }
}

Glacier2::SessionPrx
ClientSessionFactory::createGlacier2Session(
    const string& sessionId,
    const optional<Glacier2::SessionControlPrx>& ctl,
    const Ice::ConnectionPtr& con)
{
    assert(_servantManager);

    auto session = createSessionServant(sessionId);
    auto proxy = session->_register(_servantManager, con);

    if (ctl)
    {
        try
        {
            if (_filters)
            {
                Ice::Identity queryId = {"Query", _database->getInstanceName()};
                _servantManager->setSessionControl(session, *ctl, {std::move(queryId)});
            }
        }
        catch (const Ice::LocalException& e)
        {
            session->destroy();

            Ice::Warning out(_database->getTraceLevels()->logger);
            out << "Failed to callback Glacier2 session control object:\n" << e;

            throw Glacier2::CannotCreateSessionException("internal server error");
        }
    }

    // We can't use a non-0 timeout. As of Ice 3.8, heartbeats may not be sent at all on a busy connection. Furthermore,
    // as of Ice 3.8, Glacier2 no longer "converts" heartbeats into keepAlive requests.
    _reaper->add(make_shared<SessionReapable<SessionI>>(_database->getTraceLevels()->logger, session), 0s, con);
    return Ice::uncheckedCast<Glacier2::SessionPrx>(proxy);
}

shared_ptr<SessionI>
ClientSessionFactory::createSessionServant(const string& userId)
{
    return make_shared<SessionI>(userId, _database, _timer);
}

const shared_ptr<TraceLevels>&
ClientSessionFactory::getTraceLevels() const
{
    return _database->getTraceLevels();
}

ClientSessionManagerI::ClientSessionManagerI(const shared_ptr<ClientSessionFactory>& factory) : _factory(factory) {}

std::optional<Glacier2::SessionPrx>
ClientSessionManagerI::create(string user, std::optional<Glacier2::SessionControlPrx> ctl, const Ice::Current& current)
{
    return _factory->createGlacier2Session(user, ctl, current.con);
}

ClientSSLSessionManagerI::ClientSSLSessionManagerI(const shared_ptr<ClientSessionFactory>& factory) : _factory(factory)
{
}

std::optional<Glacier2::SessionPrx>
ClientSSLSessionManagerI::create(
    Glacier2::SSLInfo info,
    std::optional<Glacier2::SessionControlPrx> ctl,
    const Ice::Current& current)
{
    string userDN;
    if (!info.certs.empty()) // TODO: Require userDN?
    {
        try
        {
            Ice::SSL::ScopedCertificate cert = Ice::SSL::decodeCertificate(info.certs[0]);
            userDN = Ice::SSL::getSubjectName(cert.get());
        }
        catch (const Ice::Exception& e)
        {
            // This shouldn't happen, the SSLInfo is supposed to be encoded by Glacier2.
            Ice::Error out(_factory->getTraceLevels()->logger);
            out << "SSL session manager couldn't decode SSL certificates:\n" << e;

            throw Glacier2::CannotCreateSessionException("internal server error");
        }
    }

    return _factory->createGlacier2Session(userDN, ctl, current.con);
}
