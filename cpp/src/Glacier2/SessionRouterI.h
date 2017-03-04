// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_SESSION_ROUTER_I_H
#define GLACIER2_SESSION_ROUTER_I_H

#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>

#include <Ice/Ice.h>

#include <Glacier2/PermissionsVerifierF.h>
#include <Glacier2/Router.h>
#include <Glacier2/Instrumentation.h>

#include <set>


namespace Glacier2
{

class RouterI;
typedef IceUtil::Handle<RouterI> RouterIPtr;

class SessionRouterI;
typedef IceUtil::Handle<SessionRouterI> SessionRouterIPtr;

class FilterManager;
typedef IceUtil::Handle<FilterManager> FilterManagerPtr;

class CreateSession;
typedef IceUtil::Handle<CreateSession> CreateSessionPtr;

class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class ClientBlobject;
typedef IceUtil::Handle<ClientBlobject> ClientBlobjectPtr;

class ServerBlobject;
typedef IceUtil::Handle<ServerBlobject> ServerBlobjectPtr;

class CreateSession : public IceUtil::Shared
{
public:

    CreateSession(const SessionRouterIPtr&, const std::string&, const Ice::Current&);

    void create();
    void addPendingCallback(const CreateSessionPtr&);

    void authorized(bool);
    void unexpectedAuthorizeException(const Ice::Exception&);

    void sessionCreated(const SessionPrx&);
    void unexpectedCreateSessionException(const Ice::Exception&);

    void exception(const Ice::Exception&);

    void createException(const Ice::Exception&);

    virtual void authorize() = 0;
    virtual void createSession() = 0;
    virtual FilterManagerPtr createFilterManager() = 0;
    virtual void finished(const SessionPrx&) = 0;
    virtual void finished(const Ice::Exception&) = 0;

protected:

    const InstancePtr _instance;
    const SessionRouterIPtr _sessionRouter;
    const std::string _user;
    const Ice::Current _current;
    Ice::Context _context;
    std::vector<CreateSessionPtr> _pendingCallbacks;
    SessionControlPrx _control;
    FilterManagerPtr _filterManager;
};

class UserPasswordCreateSession;
typedef IceUtil::Handle<UserPasswordCreateSession> UserPasswordCreateSessionPtr;

class SSLCreateSession;
typedef IceUtil::Handle<SSLCreateSession> SSLCreateSessionPtr;

class SessionRouterI : public Router,
                       public Glacier2::Instrumentation::ObserverUpdater,
                       private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionRouterI(const InstancePtr&, const PermissionsVerifierPrx&, const SessionManagerPrx&,
                   const SSLPermissionsVerifierPrx&, const SSLSessionManagerPrx&);
    virtual ~SessionRouterI();
    void destroy();

    virtual Ice::ObjectPrx getClientProxy(IceUtil::Optional<bool>&, const Ice::Current&) const;
    virtual Ice::ObjectPrx getServerProxy(const Ice::Current&) const;
    virtual Ice::ObjectProxySeq addProxies(const Ice::ObjectProxySeq&, const Ice::Current&);
    virtual std::string getCategoryForClient(const Ice::Current&) const;
    virtual void createSession_async(const AMD_Router_createSessionPtr&, const std::string&, const std::string&,
                               const Ice::Current&);
    virtual void createSessionFromSecureConnection_async(const AMD_Router_createSessionFromSecureConnectionPtr&,
                                                         const Ice::Current&);
    virtual void refreshSession_async(const AMD_Router_refreshSessionPtr&, const Ice::Current&);
    virtual void destroySession(const ::Ice::Current&);
    virtual Ice::Long getSessionTimeout(const ::Ice::Current&) const;
    virtual Ice::Int getACMTimeout(const ::Ice::Current&) const;

    virtual void updateSessionObservers();

    RouterIPtr getRouter(const Ice::ConnectionPtr&, const Ice::Identity&, bool = true) const;

    Ice::ObjectPtr getClientBlobject(const Ice::ConnectionPtr&, const Ice::Identity&) const;
    Ice::ObjectPtr getServerBlobject(const std::string&) const;

    void expireSessions();

    void refreshSession(const ::Ice::ConnectionPtr&);
    void destroySession(const ::Ice::ConnectionPtr&);

    int sessionTraceLevel() const { return _sessionTraceLevel; }

private:

    RouterIPtr getRouterImpl(const Ice::ConnectionPtr&, const Ice::Identity&, bool) const;

    void sessionDestroyException(const Ice::Exception&);

    bool startCreateSession(const CreateSessionPtr&, const Ice::ConnectionPtr&);
    void finishCreateSession(const Ice::ConnectionPtr&, const RouterIPtr&);
    friend class Glacier2::CreateSession;
    friend class Glacier2::UserPasswordCreateSession;
    friend class Glacier2::SSLCreateSession;

    const InstancePtr _instance;
    const int _sessionTraceLevel;
    const int _rejectTraceLevel;
    const PermissionsVerifierPrx _verifier;
    const SessionManagerPrx _sessionManager;
    const SSLPermissionsVerifierPrx _sslVerifier;
    const SSLSessionManagerPrx _sslSessionManager;

    IceUtil::Time _sessionTimeout;
    Ice::CloseCallbackPtr _closeCallback;
    Ice::HeartbeatCallbackPtr _heartbeatCallback;

    class SessionThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
    {
    public:

        SessionThread(const SessionRouterIPtr&, const IceUtil::Time&);
        virtual ~SessionThread();
        void destroy();

        virtual void run();

    private:

        SessionRouterIPtr _sessionRouter;
        const IceUtil::Time _sessionTimeout;
    };
    typedef IceUtil::Handle<SessionThread> SessionThreadPtr;
    SessionThreadPtr _sessionThread;

    std::map<Ice::ConnectionPtr, RouterIPtr> _routersByConnection;
    mutable std::map<Ice::ConnectionPtr, RouterIPtr>::iterator _routersByConnectionHint;

    std::map<std::string, RouterIPtr> _routersByCategory;
    mutable std::map<std::string, RouterIPtr>::iterator _routersByCategoryHint;

    std::map<Ice::ConnectionPtr, CreateSessionPtr> _pending;

    Callback_Session_destroyPtr _sessionDestroyCallback;

    bool _destroy;
};

}

#endif
