// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

class GLACIER2_API SessionFactoryHelper : public IceUtil::Shared
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
typedef IceUtil::Handle<SessionFactoryHelper> SessionFactoryHelperPtr;

}

#endif
