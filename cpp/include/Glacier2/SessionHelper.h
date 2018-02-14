// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

class GLACIER2_API SessionHelper
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:
    virtual ~SessionHelper();

    virtual void destroy() = 0;
    virtual Ice::CommunicatorPtr communicator() const = 0;
    virtual std::string categoryForClient() const = 0;
    virtual Ice::ObjectPrxPtr addWithUUID(const Ice::ObjectPtr&) = 0;
    virtual SessionPrxPtr session() const = 0;
    virtual bool isConnected() const = 0;
    virtual Ice::ObjectAdapterPtr objectAdapter() = 0;

#ifndef ICE_CPP11_MAPPING
    bool operator==(const SessionHelper&) const;
    bool operator!=(const SessionHelper&) const;
#endif

};
ICE_DEFINE_PTR(SessionHelperPtr, SessionHelper);

class GLACIER2_API SessionCallback
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:
    virtual ~SessionCallback();

    virtual void createdCommunicator(const SessionHelperPtr& session) = 0;
    virtual void connected(const SessionHelperPtr&) = 0;
    virtual void disconnected(const SessionHelperPtr&) = 0;
    virtual void connectFailed(const SessionHelperPtr&, const Ice::Exception&) = 0;
};
ICE_DEFINE_PTR(SessionCallbackPtr, SessionCallback);

class SessionThreadCallback;

class GLACIER2_API SessionFactoryHelper
#ifdef ICE_CPP11_MAPPING
    : public std::enable_shared_from_this<SessionFactoryHelper>
#else
    : public virtual IceUtil::Shared
#endif
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

    void setConnectContext(const std::map<std::string, std::string>& context);

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
    std::map<std::string, std::string> _context;
    bool _useCallbacks;
    std::map<const SessionHelper*, IceUtil::ThreadPtr> _threads;
};
ICE_DEFINE_PTR(SessionFactoryHelperPtr, SessionFactoryHelper);

}

#endif
