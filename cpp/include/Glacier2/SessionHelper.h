// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

class GLACIER2_API SessionHelper : public IceUtil::Shared
{

public:

    virtual void destroy() = 0;
    virtual Ice::CommunicatorPtr communicator() const = 0;
    virtual std::string categoryForClient() const = 0;
    virtual Ice::ObjectPrx addWithUUID(const Ice::ObjectPtr&) = 0;
    virtual Glacier2::SessionPrx session() const = 0;
    virtual bool isConnected() const = 0;
    virtual Ice::ObjectAdapterPtr objectAdapter() = 0;

    bool operator==(const Glacier2::SessionHelper&) const;
    bool operator!=(const Glacier2::SessionHelper&) const;
};
typedef IceUtil::Handle<SessionHelper> SessionHelperPtr;

class GLACIER2_API SessionCallback : virtual public IceUtil::Shared
{

public:

    virtual void createdCommunicator(const SessionHelperPtr& session) = 0;
    virtual void connected(const SessionHelperPtr&) = 0;
    virtual void disconnected(const SessionHelperPtr&) = 0;
    virtual void connectFailed(const SessionHelperPtr&, const Ice::Exception&) = 0;
};
typedef IceUtil::Handle<SessionCallback> SessionCallbackPtr;

class SessionThreadCallback;

class GLACIER2_API SessionFactoryHelper : public IceUtil::Shared
{
    friend class SessionThreadCallback; // To access thread functions

public:

    SessionFactoryHelper(const SessionCallbackPtr& callback);
    SessionFactoryHelper(const Ice::InitializationData&, const SessionCallbackPtr&);
    SessionFactoryHelper(const Ice::PropertiesPtr&, const SessionCallbackPtr&);

    ~SessionFactoryHelper();

    void destroy();

    void setRouterIdentity(const Ice::Identity&);
    Ice::Identity getRouterIdentity() const;

    void setRouterHost(const std::string&);
    std::string getRouterHost() const;

    ICE_DEPRECATED_API("is deprecated, use SessionFactoryHelper::setProtocol instead")
    void setSecure(bool);
    ICE_DEPRECATED_API("is deprecated, use SessionFactoryHelper::getProtocol instead")
    bool getSecure() const;

    void setProtocol(const std::string&);
    std::string getProtocol() const;

    void setTimeout(int);
    int getTimeout() const;

    void setPort(int port);
    int getPort() const;

    Ice::InitializationData getInitializationData() const;

    void setConnectContext(std::map<std::string, std::string> context);

    void setUseCallbacks(bool);
    bool getUseCallbacks() const;

    SessionHelperPtr connect();
    SessionHelperPtr connect(const std::string&,  const std::string&);

private:

    void addThread(const SessionHelper*, const IceUtil::ThreadPtr&);

    Ice::InitializationData createInitData();
    std::string getRouterFinderStr();
    int getPortInternal() const;
    std::string createProxyStr(const Ice::Identity& ident);
    void setDefaultProperties();

    IceUtil::Mutex _mutex;
    std::string _routerHost;
    Ice::Identity _identity;
    std::string _protocol;
    int _port;
    int _timeout;
    Ice::InitializationData _initData;
    SessionCallbackPtr _callback;
    std::map< std::string, std::string> _context;
    bool _useCallbacks;
    std::map<const SessionHelper*, IceUtil::ThreadPtr> _threads;
};
typedef IceUtil::Handle<SessionFactoryHelper> SessionFactoryHelperPtr;

}

#endif
