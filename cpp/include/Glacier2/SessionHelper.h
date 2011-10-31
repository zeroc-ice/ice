// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_SESSION_HELPER_H
#define GLACIER2_SESSION_HELPER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Mutex.h>

#include <Ice/Initialize.h>
#include <Ice/Properties.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Ice.h>

#include <Glacier2/Session.h>
#include <Glacier2/Router.h>

#include <map>
#include <string>

namespace Glacier2
{

const int GLACIER2_SSL_PORT = 4064;
const int GLACIER2_TCP_PORT = 4063;

class SessionCallback;
typedef IceUtil::Handle<SessionCallback> SessionCallbackPtr;

class SessionHelper;
typedef IceUtil::Handle<SessionHelper> SessionHelperPtr;

class SessionFactoryHelper;
typedef IceUtil::Handle<SessionFactoryHelper> SessionFactoryHelperPtr;

class ConnectThread;
class DestroyInternal;

class SessionRefreshThread : public IceUtil::Thread
{

public:
    
    SessionRefreshThread(const SessionHelperPtr&, const Glacier2::RouterPrx&, long period);
    virtual void run();
    void done();
    void success();
    void failure(const Ice::Exception&);
    
private:
    
    const Glacier2::Callback_Router_refreshSessionPtr _cb;
    const SessionHelperPtr _session;
    const Glacier2::RouterPrx _router;
    long _period;
    bool _done;
    IceUtil::Monitor<IceUtil::Mutex> _monitor;
};
typedef IceUtil::Handle<SessionRefreshThread> SessionRefreshThreadPtr;

class SessionCallback : public IceUtil::Shared
{

public:
    
    virtual ~SessionCallback(){}
    virtual void createdCommunicator(const SessionHelperPtr& session) = 0;
    virtual void connected(const SessionHelperPtr&) = 0;
    virtual void disconnected(const SessionHelperPtr&) = 0;
    virtual void connectFailed(const SessionHelperPtr&, const Ice::Exception&) = 0;
};

class ConnectStrategy : public IceUtil::Shared
{

public:
    
    virtual Glacier2::SessionPrx connect(const Glacier2::RouterPrx& router) = 0;    
};
typedef IceUtil::Handle< ConnectStrategy> ConnectStrategyPtr;

class SessionHelper : public IceUtil::Shared
{
    
public:
    
    SessionHelper(const SessionCallbackPtr&, const Ice::InitializationData&);
    void destroy();
    Ice::CommunicatorPtr communicator() const;
    std::string categoryForClient() const;
    Ice::ObjectPrx addWithUUID(const Ice::ObjectPtr&);
    Glacier2::SessionPrx session() const;
    bool isConnected() const;
    Ice::ObjectAdapterPtr objectAdapter();
    
    friend class Glacier2::DestroyInternal;
    friend class Glacier2::ConnectThread;
    friend class Glacier2::SessionFactoryHelper;
    
     bool operator==(const SessionHelper&) const;
     bool operator!=(const SessionHelper&) const;

private:
    
    void destroy(const IceUtil::ThreadPtr&);

    Ice::ObjectAdapterPtr internalObjectAdapter();
    void connected(const RouterPrx&, const SessionPrx&);
    void destroyInternal(const Ice::DispatcherCallPtr&);
    
    void connect(const std::map<std::string, std::string>&);
    void connect(const std::string&, const std::string&, const std::map<std::string, std::string>&);
    
    void connectImpl(const ConnectStrategyPtr&);
    void dispatchCallback(const Ice::DispatcherCallPtr&, const Ice::ConnectionPtr&);
    void dispatchCallbackAndWait(const Ice::DispatcherCallPtr&, const Ice::ConnectionPtr&);

    IceUtil::Mutex _mutex;
    Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _adapter;
    Glacier2::RouterPrx _router;
    Glacier2::SessionPrx _session;
    SessionRefreshThreadPtr _refreshThread;
    std::string _category;
    bool _connected;
    bool _destroy;
    Ice::InitializationData _initData;
    SessionCallbackPtr _callback;
};

class SessionFactoryHelper : public IceUtil::Shared
{

public:
    
    SessionFactoryHelper(const SessionCallbackPtr& callback);
    SessionFactoryHelper(const Ice::InitializationData&, const SessionCallbackPtr&);
    SessionFactoryHelper(const Ice::PropertiesPtr&, const SessionCallbackPtr&);

    void setRouterIdentity(const Ice::Identity&);
    Ice::Identity getRouterIdentity() const;
    
    void setRouterHost(const std::string&);
    std::string getRouterHost() const;
    
    void setSecure(bool);
    bool getSecure() const;
    
    void setTimeout(int);
    int getTimeout() const;
    
    void setPort(int port);
    int getPort() const;
    
    Ice::InitializationData getInitializationData() const;

    void setConnectContext(std::map<std::string, std::string> context);

    SessionHelperPtr connect();
    SessionHelperPtr connect(const std::string&,  const std::string&);

private:
    
    Ice::InitializationData createInitData();
    void setDefaultProperties();

    IceUtil::Mutex _mutex;
    std::string _routerHost;
    Ice::Identity _identity;
    bool _secure;
    int _port;
    int _timeout;
    Ice::InitializationData _initData;
    SessionCallbackPtr _callback;
    std::map< std::string, std::string> _context;
};

}

#endif
