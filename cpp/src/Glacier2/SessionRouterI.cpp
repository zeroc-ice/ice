
// Copyright (c) ZeroC, Inc.

#include "SessionRouterI.h"
#include "../Ice/SSL/SSLUtil.h"
#include "FilterManager.h"
#include "Glacier2/PermissionsVerifier.h"
#include "Ice/Ice.h"
#include "RouterI.h"

using namespace std;
using namespace Ice;
using namespace Glacier2;

namespace
{
    shared_ptr<IPConnectionInfo> getIPConnectionInfo(const ConnectionInfoPtr& info)
    {
        for (ConnectionInfoPtr p = info; p; p = p->underlying)
        {
            auto ipInfo = dynamic_pointer_cast<IPConnectionInfo>(p);
            if (ipInfo)
            {
                return ipInfo;
            }
        }

        return nullptr;
    }
}

namespace Glacier2
{
    class SessionControlI final : public SessionControl
    {
    public:
        SessionControlI(
            shared_ptr<SessionRouterI> sessionRouter,
            ConnectionPtr connection,
            shared_ptr<FilterManager> filterManager)
            : _sessionRouter(std::move(sessionRouter)),
              _connection(std::move(connection)),
              _filters(std::move(filterManager))
        {
        }

        optional<StringSetPrx> categories(const Current&) final { return _filters->categoriesPrx(); }

        optional<StringSetPrx> adapterIds(const Current&) final { return _filters->adapterIdsPrx(); }

        optional<IdentitySetPrx> identities(const Current&) final { return _filters->identitiesPrx(); }

        int32_t getSessionTimeout(const Current&) final
        {
            return static_cast<int32_t>(_sessionRouter->getSessionTimeout());
        }

        void destroy(const Current&) final
        {
            _sessionRouter->destroySession(
                _connection,
                [defaultExceptionHandler = _sessionRouter->defaultSessionDestroyExceptionHandler()](exception_ptr e)
                {
                    try
                    {
                        rethrow_exception(e);
                    }
                    catch (const Ice::ObjectNotExistException&)
                    {
                        // Ignored. This typically occurs when the application-provided session calls
                        // SessionControl::destroy in its own destroy implementation.
                    }
                    catch (...)
                    {
                        defaultExceptionHandler(e);
                    }
                });

            _filters->destroy();

            // Initiate a graceful closure of the connection. Only initiate and graceful because the ultimate caller
            // can be the Glacier2 client calling us over _connection.
            _connection->close(nullptr, nullptr);
        }

    private:
        const shared_ptr<SessionRouterI> _sessionRouter;
        const ConnectionPtr _connection;
        const shared_ptr<FilterManager> _filters;
    };

    class UserPasswordCreateSession final : public CreateSession
    {
    public:
        UserPasswordCreateSession(
            function<void(const optional<SessionPrx>&)> response,
            function<void(exception_ptr)> exception,
            const string& user,
            string password,
            const Ice::Current& current,
            const shared_ptr<SessionRouterI>& sessionRouter)
            : CreateSession(sessionRouter, user, current),
              _response(std::move(response)),
              _exception(std::move(exception)),
              _password(std::move(password))
        {
        }

        void checkPermissionsResponse(bool ok, const string& reason)
        {
            if (ok)
            {
                authorized(_sessionRouter->_sessionManager != nullopt);
            }
            else
            {
                exception(make_exception_ptr(
                    PermissionDeniedException(reason.empty() ? string("permission denied") : reason)));
            }
        }

        void checkPermissionsException(exception_ptr ex)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const PermissionDeniedException&)
            {
                exception(ex);
            }
            catch (...)
            {
                unexpectedAuthorizeException(ex);
            }
        }

        void authorize() override
        {
            assert(_sessionRouter->_verifier);

            auto ctx = _current.ctx;
            ctx.insert(_context.begin(), _context.end());
            auto self = static_pointer_cast<UserPasswordCreateSession>(shared_from_this());
            _sessionRouter->_verifier->checkPermissionsAsync(
                _user,
                _password,
                [self](bool ok, const string& reason) { self->checkPermissionsResponse(ok, reason); },
                [self](exception_ptr e) { self->checkPermissionsException(e); },
                nullptr,
                ctx);
        }

        shared_ptr<FilterManager> createFilterManager() final { return FilterManager::create(_instance, _user, true); }

        void createSession() final
        {
            auto ctx = _current.ctx;
            ctx.insert(_context.begin(), _context.end());
            auto self = shared_from_this();
            _sessionRouter->_sessionManager->createAsync(
                _user,
                _control,
                [self](optional<SessionPrx> session)
                {
                    if (session)
                    {
                        self->sessionCreated(std::move(session));
                    }
                    else
                    {
                        self->exception(make_exception_ptr(
                            CannotCreateSessionException("Session manager returned a null session proxy")));
                    }
                },
                [self](exception_ptr e) { self->createException(e); },
                nullptr,
                ctx);
        }

        void finished(optional<SessionPrx> session) final { _response(std::move(session)); }

        void finished(exception_ptr ex) final { _exception(ex); }

    private:
        const function<void(optional<SessionPrx>)> _response;
        const function<void(exception_ptr)> _exception;
        const string _password;
    };

    class SSLCreateSession final : public CreateSession
    {
    public:
        SSLCreateSession(
            function<void(const optional<SessionPrx>& returnValue)> response,
            function<void(exception_ptr)> exception,
            const string& user,
            SSLInfo sslInfo,
            const Ice::Current& current,
            const shared_ptr<SessionRouterI>& sessionRouter)
            : CreateSession(sessionRouter, user, current),
              _response(std::move(response)),
              _exception(std::move(exception)),
              _sslInfo(std::move(sslInfo))
        {
        }

        void authorizeResponse(bool ok, const string& reason)
        {
            if (ok)
            {
                authorized(_sessionRouter->_sslSessionManager != nullopt);
            }
            else
            {
                exception(make_exception_ptr(
                    PermissionDeniedException(reason.empty() ? string("permission denied") : reason)));
            }
        }

        void authorizeException(exception_ptr ex)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const PermissionDeniedException&)
            {
                exception(ex);
            }
            catch (...)
            {
                unexpectedAuthorizeException(ex);
            }
        }

        void authorize() override
        {
            assert(_sessionRouter->_sslVerifier);

            auto ctx = _current.ctx;
            ctx.insert(_context.begin(), _context.end());

            auto self = static_pointer_cast<SSLCreateSession>(shared_from_this());
            _sessionRouter->_sslVerifier->authorizeAsync(
                _sslInfo,
                [self](bool ok, const string& reason) { self->authorizeResponse(ok, reason); },
                [self](exception_ptr e) { self->authorizeException(e); },
                nullptr,
                ctx);
        }

        shared_ptr<FilterManager> createFilterManager() final { return FilterManager::create(_instance, _user, false); }

        void createSession() final
        {
            auto ctx = _current.ctx;
            ctx.insert(_context.begin(), _context.end());

            auto self = static_pointer_cast<SSLCreateSession>(shared_from_this());
            _sessionRouter->_sslSessionManager->createAsync(
                _sslInfo,
                _control,
                [self](optional<SessionPrx> session)
                {
                    if (session)
                    {
                        self->sessionCreated(std::move(session));
                    }
                    else
                    {
                        self->exception(make_exception_ptr(
                            CannotCreateSessionException("Session manager returned a null session proxy")));
                    }
                },
                [self](exception_ptr e) { self->createException(e); },
                nullptr,
                ctx);
        }

        void finished(optional<SessionPrx> session) final { _response(std::move(session)); }

        void finished(exception_ptr ex) final { _exception(ex); }

    private:
        const function<void(const optional<SessionPrx>)> _response;
        const function<void(exception_ptr)> _exception;
        const SSLInfo _sslInfo;
    };
}

CreateSession::CreateSession(shared_ptr<SessionRouterI> sessionRouter, string user, const Ice::Current& current)
    : _instance(sessionRouter->_instance),
      _sessionRouter(std::move(sessionRouter)),
      _user(std::move(user)),
      _current(current)
{
    // Clear reserved contexts potentially set by client
    auto ctx = _current.ctx;
    ctx.erase("_con.type");
    ctx.erase("_con.remotePort");
    ctx.erase("_con.remoteAddress");
    ctx.erase("_con.localPort");
    ctx.erase("_con.localAddress");
    ctx.erase("_con.peerCert");
    const_cast<Ice::Current&>(_current).ctx = ctx;

    if (_instance->properties()->getIcePropertyAsInt("Glacier2.AddConnectionContext") > 0)
    {
        _context["_con.type"] = current.con->type();
        {
            auto info = getIPConnectionInfo(current.con->getInfo());
            if (info)
            {
                ostringstream os;
                os << info->remotePort;
                _context["_con.remotePort"] = os.str();
                _context["_con.remoteAddress"] = info->remoteAddress;
                os.str("");
                os << info->localPort;
                _context["_con.localPort"] = os.str();
                _context["_con.localAddress"] = info->localAddress;
            }
        }
        {
            auto info = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(current.con->getInfo());
            if (info && info->peerCertificate)
            {
                _context["_con.peerCert"] = Ice::SSL::encodeCertificate(info->peerCertificate);
            }
        }
    }
}

void
CreateSession::create()
{
    try
    {
        if (_sessionRouter->startCreateSession(shared_from_this(), _current.con))
        {
            authorize();
        }
    }
    catch (const Ice::Exception&)
    {
        finished(current_exception());
    }
}

void
CreateSession::addPendingCallback(shared_ptr<CreateSession> callback)
{
    _pendingCallbacks.push_back(std::move(callback));
}

void
CreateSession::authorized(bool createSession)
{
    //
    // Create the filter manager now as it's required for the session control object.
    //
    _filterManager = createFilterManager();

    //
    // If we have a session manager configured, we create a client-visible session object,
    // otherwise, we return a null session proxy.
    //
    if (createSession)
    {
        if (_instance->serverObjectAdapter())
        {
            auto obj = make_shared<SessionControlI>(_sessionRouter, _current.con, _filterManager);
            _control = _instance->serverObjectAdapter()->addWithUUID<SessionControlPrx>(obj);
        }
        this->createSession();
    }
    else
    {
        sessionCreated(nullopt);
    }
}

void
CreateSession::unexpectedAuthorizeException(exception_ptr ex)
{
    if (_sessionRouter->sessionTraceLevel() >= 1)
    {
        try
        {
            rethrow_exception(ex);
        }
        catch (const Ice::Exception& e)
        {
            Warning out(_instance->logger());
            out << "exception while verifying permissions:\n" << e;
        }
    }

    exception(make_exception_ptr(PermissionDeniedException("internal server error")));
}

void
CreateSession::createException(exception_ptr ex)
{
    try
    {
        rethrow_exception(ex);
    }
    catch (const CannotCreateSessionException&)
    {
        exception(current_exception());
    }
    catch (const Ice::Exception&)
    {
        unexpectedCreateSessionException(current_exception());
    }
}

void
CreateSession::sessionCreated(optional<SessionPrx> session)
{
    //
    // Create the session router object.
    //
    shared_ptr<RouterI> router;
    try
    {
        Ice::Identity ident;
        if (_control)
        {
            ident = _control->ice_getIdentity();
        }

        if (_instance->properties()->getIcePropertyAsInt("Glacier2.AddConnectionContext") == 1)
        {
            router = make_shared<RouterI>(_instance, _current.con, _user, session, ident, _filterManager, _context);
        }
        else
        {
            router =
                make_shared<RouterI>(_instance, _current.con, _user, session, ident, _filterManager, Ice::Context());
        }
    }
    catch (const Ice::Exception&)
    {
        if (session)
        {
            session->destroyAsync(nullptr); // don't wait for response
        }
        unexpectedCreateSessionException(current_exception());
        return;
    }

    //
    // Notify the router that the creation is finished.
    //
    try
    {
        _sessionRouter->finishCreateSession(_current.con, router);
        finished(std::move(session));
    }
    catch (const Ice::Exception&)
    {
        finished(current_exception());
    }

    for (const auto& callback : _pendingCallbacks)
    {
        callback->create();
    }
}

void
CreateSession::unexpectedCreateSessionException(exception_ptr ex)
{
    if (_sessionRouter->sessionTraceLevel() >= 1)
    {
        try
        {
            rethrow_exception(ex);
        }
        catch (const Ice::Exception& e)
        {
            Trace out(_instance->logger(), "Glacier2");
            out << "exception while creating session with session manager:\n" << e;
        }
    }
    exception(make_exception_ptr(CannotCreateSessionException("internal server error")));
}

void
CreateSession::exception(exception_ptr ex)
{
    try
    {
        _sessionRouter->finishCreateSession(_current.con, nullptr);
    }
    catch (const Ice::Exception&)
    {
    }

    finished(ex);

    if (_control)
    {
        try
        {
            _instance->serverObjectAdapter()->remove(_control->ice_getIdentity());
        }
        catch (const Exception&)
        {
        }
    }

    for (const auto& callback : _pendingCallbacks)
    {
        callback->create();
    }
}

SessionRouterI::SessionRouterI(
    shared_ptr<Instance> instance,
    optional<PermissionsVerifierPrx> verifier,
    optional<SessionManagerPrx> sessionManager,
    optional<SSLPermissionsVerifierPrx> sslVerifier,
    optional<SSLSessionManagerPrx> sslSessionManager)
    : _instance(std::move(instance)),
      _sessionTraceLevel(_instance->properties()->getIcePropertyAsInt("Glacier2.Trace.Session")),
      _rejectTraceLevel(_instance->properties()->getIcePropertyAsInt("Glacier2.Client.Trace.Reject")),
      _verifier(std::move(verifier)),
      _sessionManager(std::move(sessionManager)),
      _sslVerifier(std::move(sslVerifier)),
      _sslSessionManager(std::move(sslSessionManager)),
      _routersByConnectionHint(_routersByConnection.cend()),
      _routersByCategoryHint(_routersByCategory.cend())
{
}

SessionRouterI::~SessionRouterI()
{
    lock_guard<mutex> lg(_mutex);

    assert(_destroy);
    assert(_routersByConnection.empty());
    assert(_routersByCategory.empty());
    assert(_pending.empty());
}

void
SessionRouterI::destroy()
{
    map<ConnectionPtr, shared_ptr<RouterI>> routers;
    {
        lock_guard<mutex> lg(_mutex);

        assert(!_destroy);
        _destroy = true;

        _routersByConnection.swap(routers);
        _routersByConnectionHint = _routersByConnection.cend();

        _routersByCategory.clear();
        _routersByCategoryHint = _routersByCategory.cend();
    }

    //
    // We destroy the routers outside the thread synchronization to
    // avoid deadlocks.
    //
    for (const auto& router : routers)
    {
        router.second->destroy([self = shared_from_this()](exception_ptr e) { self->sessionDestroyException(e); });
    }
}

void
SessionRouterI::getClientProxyAsync(
    std::function<void(const optional<Ice::ObjectPrx>&, optional<bool>)> response,
    std::function<void(exception_ptr)>,
    const Current& current) const
{
    // Forward to the per-client router.
    optional<bool> hasRoutingTable;
    optional<ObjectPrx> proxy = getRouter(current.con, current.id)->getClientProxy(hasRoutingTable, current);
    response(proxy, hasRoutingTable);
}

void
SessionRouterI::getServerProxyAsync(
    std::function<void(const optional<Ice::ObjectPrx>&)> response,
    std::function<void(exception_ptr)>,
    const Current& current) const
{
    // Forward to the per-client router.
    response(getRouter(current.con, current.id)->getServerProxy(current));
}

void
SessionRouterI::addProxiesAsync(
    ObjectProxySeq proxies,
    std::function<void(const ObjectProxySeq&)> response,
    std::function<void(exception_ptr)>,
    const Current& current)
{
    // Forward to the per-client router.
    response(getRouter(current.con, current.id)->addProxies(std::move(proxies), current));
}

void
SessionRouterI::getCategoryForClientAsync(
    std::function<void(string_view)> response,
    std::function<void(exception_ptr)>,
    const Ice::Current& current) const
{
    // Forward to the per-client router.
    response(getRouter(current.con, current.id)->getCategoryForClient(current));
}

void
SessionRouterI::createSessionAsync(
    string userId,
    string password,
    function<void(const optional<SessionPrx>&)> response,
    function<void(exception_ptr)> exception,
    const Current& current)
{
    if (!_verifier)
    {
        exception(make_exception_ptr(PermissionDeniedException("no configured permissions verifier")));
        return;
    }

    auto session = make_shared<UserPasswordCreateSession>(
        std::move(response),
        std::move(exception),
        std::move(userId),
        std::move(password),
        current,
        shared_from_this());
    session->create();
}

void
SessionRouterI::createSessionFromSecureConnectionAsync(
    function<void(const optional<SessionPrx>&)> response,
    function<void(std::exception_ptr)> exception,
    const Current& current)
{
    if (!_sslVerifier)
    {
        exception(make_exception_ptr(PermissionDeniedException("no configured ssl permissions verifier")));
        return;
    }

    string userDN;
    SSLInfo sslinfo;

    //
    // Populate the SSL context information.
    //
    try
    {
        auto info = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(current.con->getInfo());
        if (!info)
        {
            exception(make_exception_ptr(PermissionDeniedException("not ssl connection")));
            return;
        }

        auto ipInfo = getIPConnectionInfo(info);
        sslinfo.remotePort = ipInfo->remotePort;
        sslinfo.remoteHost = ipInfo->remoteAddress;
        sslinfo.localPort = ipInfo->localPort;
        sslinfo.localHost = ipInfo->localAddress;

        if (info->peerCertificate)
        {
            sslinfo.certs.push_back(Ice::SSL::encodeCertificate(info->peerCertificate));
            userDN = Ice::SSL::getSubjectName(info->peerCertificate);
        }
    }
    catch (const Ice::SSL::CertificateEncodingException&)
    {
        exception(make_exception_ptr(PermissionDeniedException("certificate encoding exception")));
        return;
    }
    catch (const Ice::LocalException&)
    {
        exception(make_exception_ptr(PermissionDeniedException("connection exception")));
        return;
    }

    auto session = make_shared<SSLCreateSession>(
        std::move(response),
        std::move(exception),
        userDN,
        sslinfo,
        current,
        shared_from_this());
    session->create();
}

void
SessionRouterI::destroySessionAsync(function<void()> response, function<void(exception_ptr)>, const Current& current)
{
    destroySession(current.con, defaultSessionDestroyExceptionHandler());
    response(); // We don't wait until the application-provided session is destroyed.
}

void
SessionRouterI::destroySession(const ConnectionPtr& connection, function<void(exception_ptr)> error)
{
    shared_ptr<RouterI> router;

    {
        lock_guard<mutex> lg(_mutex);

        if (_destroy)
        {
            throw ObjectNotExistException{__FILE__, __LINE__};
        }

        map<ConnectionPtr, shared_ptr<RouterI>>::const_iterator p;

        if (_routersByConnectionHint != _routersByConnection.cend() && _routersByConnectionHint->first == connection)
        {
            p = _routersByConnectionHint;
        }
        else
        {
            p = _routersByConnection.find(connection);
        }

        if (p == _routersByConnection.cend())
        {
            throw SessionNotExistException();
        }

        router = p->second;

        _routersByConnection.erase(p++);
        _routersByConnectionHint = p;

        if (_instance->serverObjectAdapter())
        {
            string category = router->serverProxy()->ice_getIdentity().category;
            assert(!category.empty());
            _routersByCategory.erase(category);
            _routersByCategoryHint = _routersByCategory.cend();
        }
    }

    // We destroy the router (and application-provided session) outside the thread synchronization, to avoid deadlocks.
    if (_sessionTraceLevel >= 1)
    {
        Trace out(_instance->logger(), "Glacier2");
        out << "destroying session\n" << router->toString();
    }

    router->destroy(std::move(error));
}

function<void(std::exception_ptr)>
SessionRouterI::defaultSessionDestroyExceptionHandler() const
{
    return [self = shared_from_this()](exception_ptr e) { self->sessionDestroyException(e); };
}

void
SessionRouterI::getSessionTimeoutAsync(
    function<void(int64_t)> response,
    function<void(exception_ptr)>,
    const Ice::Current&) const
{
    response(getSessionTimeout());
}

void
SessionRouterI::getACMTimeoutAsync(function<void(int32_t)> response, function<void(exception_ptr)>, const Ice::Current&)
    const
{
    response(getACMTimeout());
}

int64_t
SessionRouterI::getSessionTimeout() const
{
    return getACMTimeout();
}

int32_t
SessionRouterI::getACMTimeout() const
{
    int32_t idleTimeout = _instance->properties()->getIcePropertyAsInt("Ice.Connection.Server.IdleTimeout");
    return _instance->properties()->getPropertyAsIntWithDefault("Glacier2.Client.Connection.IdleTimeout", idleTimeout);
}

void
SessionRouterI::updateSessionObservers()
{
    lock_guard<mutex> lg(_mutex);

    const auto& observer = _instance->getObserver();
    assert(observer);

    for (const auto& router : _routersByConnection)
    {
        router.second->updateObserver(observer);
    }
}

shared_ptr<RouterI>
SessionRouterI::getRouter(const ConnectionPtr& connection, const Ice::Identity& id, bool close) const
{
    lock_guard<mutex> lg(_mutex);
    return getRouterImpl(connection, id, close);
}

ObjectPtr
SessionRouterI::getClientBlobject(const ConnectionPtr& connection, const Ice::Identity& id) const
{
    lock_guard<mutex> lg(_mutex);
    return getRouterImpl(connection, id, true)->getClientBlobject();
}

ObjectPtr
SessionRouterI::getServerBlobject(const string& category) const
{
    lock_guard<mutex> lg(_mutex);

    if (_destroy)
    {
        throw ObjectNotExistException{__FILE__, __LINE__};
    }

    if (_routersByCategoryHint != _routersByCategory.cend() && _routersByCategoryHint->first == category)
    {
        return _routersByCategoryHint->second->getServerBlobject();
    }

    auto p = _routersByCategory.find(category);

    if (p != _routersByCategory.cend())
    {
        _routersByCategoryHint = p;
        return p->second->getServerBlobject();
    }
    else
    {
        throw ObjectNotExistException{__FILE__, __LINE__};
    }
}

shared_ptr<RouterI>
SessionRouterI::getRouterImpl(const ConnectionPtr& connection, const Ice::Identity& id, bool close) const
{
    //
    // The connection can be null if the client tries to forward requests to
    // a proxy which points to the client endpoints (in which case the request
    // is forwarded with collocation optimization).
    //
    if (_destroy || !connection)
    {
        throw ObjectNotExistException{__FILE__, __LINE__};
    }

    if (_routersByConnectionHint != _routersByConnection.cend() && _routersByConnectionHint->first == connection)
    {
        return _routersByConnectionHint->second;
    }

    auto p = _routersByConnection.find(connection);

    if (p != _routersByConnection.cend())
    {
        _routersByConnectionHint = p;
        return p->second;
    }
    else if (close)
    {
        if (_rejectTraceLevel >= 1)
        {
            Trace out(_instance->logger(), "Glacier2");
            out << "rejecting request, no session is associated with the connection.\n";
            out << "identity: " << identityToString(id);
        }
        connection->abort();
        throw ObjectNotExistException{__FILE__, __LINE__};
    }
    return nullptr;
}

void
SessionRouterI::sessionDestroyException(exception_ptr ex) const
{
    if (_sessionTraceLevel > 0)
    {
        try
        {
            rethrow_exception(ex);
        }
        catch (const Ice::Exception& e)
        {
            Trace out(_instance->logger(), "Glacier2");
            out << "exception while destroying session\n" << e;
        }
    }
}

bool
SessionRouterI::startCreateSession(const shared_ptr<CreateSession>& cb, const ConnectionPtr& connection)
{
    lock_guard<mutex> lg(_mutex);

    if (_destroy)
    {
        throw CannotCreateSessionException("router is shutting down");
    }

    //
    // Check whether a session already exists for the connection.
    //
    {
        map<ConnectionPtr, shared_ptr<RouterI>>::const_iterator p;
        if (_routersByConnectionHint != _routersByConnection.cend() && _routersByConnectionHint->first == connection)
        {
            p = _routersByConnectionHint;
        }
        else
        {
            p = _routersByConnection.find(connection);
        }

        if (p != _routersByConnection.cend())
        {
            throw CannotCreateSessionException("session exists");
        }
    }

    auto p = _pending.find(connection);
    if (p != _pending.end())
    {
        //
        // If some other thread is currently trying to create a
        // session, we wait until this thread is finished.
        //
        p->second->addPendingCallback(cb);
        return false;
    }
    else
    {
        //
        // No session exists yet, so we will try to create one. To
        // avoid that other threads try to create sessions for the
        // same connection, we add our endpoints to _pending.
        //
        _pending.insert(make_pair(connection, cb));
        return true;
    }
}

void
SessionRouterI::finishCreateSession(const ConnectionPtr& connection, const shared_ptr<RouterI>& router)
{
    lock_guard<mutex> lg(_mutex);

    //
    // Signal other threads that we are done with trying to
    // establish a session for our connection;
    //
    _pending.erase(connection);

    if (!router)
    {
        return;
    }

    if (_destroy)
    {
        router->destroy([self = shared_from_this()](exception_ptr e) { self->sessionDestroyException(e); });

        throw CannotCreateSessionException("router is shutting down");
    }

    _routersByConnectionHint = _routersByConnection.insert(_routersByConnectionHint, {connection, router});

    if (_instance->serverObjectAdapter())
    {
        string category = router->serverProxy()->ice_getIdentity().category;
        assert(!category.empty());
        auto rc = _routersByCategory.insert({category, router});
        assert(rc.second);
        _routersByCategoryHint = rc.first;
    }

    connection->setCloseCallback(
        [self = shared_from_this()](const ConnectionPtr& c)
        {
            try
            {
                self->destroySession(c, self->defaultSessionDestroyExceptionHandler());
            }
            catch (const std::exception&)
            {
            }
        });

    if (_sessionTraceLevel >= 1)
    {
        Trace out(_instance->logger(), "Glacier2");
        out << "created session\n" << router->toString();
    }
}
