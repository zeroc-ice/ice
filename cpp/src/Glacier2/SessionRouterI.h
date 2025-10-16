// Copyright (c) ZeroC, Inc.

#ifndef GLACIER2_SESSION_ROUTER_I_H
#define GLACIER2_SESSION_ROUTER_I_H

#include "Glacier2/PermissionsVerifier.h"
#include "Glacier2/Router.h"
#include "Ice/Ice.h"
#include "Instrumentation.h"

#include <functional>
#include <set>

namespace Glacier2
{
    class FilterManager;
    class Instance;
    class RouterI;
    class SessionRouterI;
    class SSLCreateSession;
    class UserPasswordCreateSession;

    class CreateSession : public std::enable_shared_from_this<CreateSession>
    {
    public:
        CreateSession(std::shared_ptr<SessionRouterI>, std::string, const Ice::Current&);

        void create();
        void addPendingCallback(std::shared_ptr<CreateSession>);

        void authorized(bool);
        void unexpectedAuthorizeException(std::exception_ptr);

        void sessionCreated(std::optional<SessionPrx>);
        void unexpectedCreateSessionException(std::exception_ptr);

        void exception(std::exception_ptr);

        void createException(std::exception_ptr);

        virtual void authorize() = 0;
        virtual void createSession() = 0;
        virtual std::shared_ptr<FilterManager> createFilterManager() = 0;
        virtual void finished(std::optional<SessionPrx>) = 0;
        virtual void finished(std::exception_ptr) = 0;

    protected:
        const std::shared_ptr<Instance> _instance;
        const std::shared_ptr<SessionRouterI> _sessionRouter;
        const std::string _user;
        const Ice::Current _current;
        Ice::Context _context;
        std::vector<std::shared_ptr<CreateSession>> _pendingCallbacks;
        std::optional<SessionControlPrx> _control;
        std::shared_ptr<FilterManager> _filterManager;
    };

    class SessionRouterI final : public AsyncRouter,
                                 public Glacier2::Instrumentation::ObserverUpdater,
                                 public std::enable_shared_from_this<SessionRouterI>
    {
    public:
        SessionRouterI(
            std::shared_ptr<Instance>,
            std::optional<PermissionsVerifierPrx>,
            std::optional<SessionManagerPrx>,
            std::optional<SSLPermissionsVerifierPrx>,
            std::optional<SSLSessionManagerPrx>);
        ~SessionRouterI() final;
        void destroy();

        void getClientProxyAsync(
            std::function<void(const std::optional<Ice::ObjectPrx>& returnValue, std::optional<bool> hasRoutingTable)>
                response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void getServerProxyAsync(
            std::function<void(const std::optional<Ice::ObjectPrx>& returnValue)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void addProxiesAsync(
            Ice::ObjectProxySeq,
            std::function<void(const Ice::ObjectProxySeq&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void getCategoryForClientAsync(
            std::function<void(std::string_view)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void createSessionAsync(
            std::string,
            std::string,
            std::function<void(const std::optional<SessionPrx>&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        void createSessionFromSecureConnectionAsync(
            std::function<void(const std::optional<SessionPrx>&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        void refreshSessionAsync(
            std::function<void()> response,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final
        {
            response();
        }

        void destroySessionAsync(
            std::function<void()> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void getSessionTimeoutAsync(
            std::function<void(std::int64_t)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void getACMTimeoutAsync(
            std::function<void(std::int32_t)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        std::int64_t getSessionTimeout() const;
        std::int32_t getACMTimeout() const;

        void updateSessionObservers() final;

        [[nodiscard]] std::shared_ptr<RouterI>
        getRouter(const Ice::ConnectionPtr&, const Ice::Identity&, bool = true) const;

        [[nodiscard]] Ice::ObjectPtr getClientBlobject(const Ice::ConnectionPtr&, const Ice::Identity&) const;
        [[nodiscard]] Ice::ObjectPtr getServerBlobject(const std::string&) const;

        /// Destroys the Glacier2 internal session.
        /// @param connection The client->Glacier2 router connection that identifies the session to destroy.
        /// @param error A callback that the implementation calls when the destruction of the application-provided
        /// session fails (see SessionManager).
        void destroySession(const Ice::ConnectionPtr& connection, std::function<void(std::exception_ptr)> error);

        [[nodiscard]] int sessionTraceLevel() const { return _sessionTraceLevel; }

        /// Provides the default handling of exceptions thrown by the destruction of application-provided sessions.
        [[nodiscard]] std::function<void(std::exception_ptr)> defaultSessionDestroyExceptionHandler() const;

    private:
        void sessionDestroyException(std::exception_ptr) const;

        [[nodiscard]] std::shared_ptr<RouterI>
        getRouterImpl(const Ice::ConnectionPtr&, const Ice::Identity&, bool) const;

        bool startCreateSession(const std::shared_ptr<CreateSession>&, const Ice::ConnectionPtr&);
        void finishCreateSession(const Ice::ConnectionPtr&, const std::shared_ptr<RouterI>&);

        friend class Glacier2::CreateSession;
        friend class Glacier2::UserPasswordCreateSession;
        friend class Glacier2::SSLCreateSession;

        const std::shared_ptr<Instance> _instance;
        const int _sessionTraceLevel;
        const int _rejectTraceLevel;
        const std::optional<PermissionsVerifierPrx> _verifier;
        const std::optional<SessionManagerPrx> _sessionManager;
        const std::optional<SSLPermissionsVerifierPrx> _sslVerifier;
        const std::optional<SSLSessionManagerPrx> _sslSessionManager;

        std::map<Ice::ConnectionPtr, std::shared_ptr<RouterI>> _routersByConnection;
        mutable std::map<Ice::ConnectionPtr, std::shared_ptr<RouterI>>::const_iterator _routersByConnectionHint;

        std::map<std::string, std::shared_ptr<RouterI>> _routersByCategory;
        mutable std::map<std::string, std::shared_ptr<RouterI>>::const_iterator _routersByCategoryHint;

        std::map<Ice::ConnectionPtr, std::shared_ptr<CreateSession>> _pending;

        bool _destroy{false};

        mutable std::mutex _mutex;
    };
}

#endif
