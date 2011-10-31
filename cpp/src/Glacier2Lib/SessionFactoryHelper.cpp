// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/SessionHelper.h>

using namespace std;

Glacier2::SessionFactoryHelper::SessionFactoryHelper(const SessionCallbackPtr& callback) :
    _routerHost("127.0.0.1"),
    _secure(true),
    _port(0),
    _timeout(10000),
    _callback(callback)
{
    _identity.name = "router";
    _identity.category = "Glacier2";
    _initData.properties = Ice::createProperties();
    setDefaultProperties();
}

Glacier2::SessionFactoryHelper::SessionFactoryHelper(const Ice::InitializationData& initData,
                                           const SessionCallbackPtr& callback) :
    _routerHost("127.0.0.1"),
    _secure(true),
    _port(0),
    _timeout(10000),
    _initData(initData),
    _callback(callback)
{
    _identity.name = "router";
    _identity.category = "Glacier2";
    if(!initData.properties)
    {
        _initData.properties = Ice::createProperties();
    }
    setDefaultProperties();
}

Glacier2::SessionFactoryHelper::SessionFactoryHelper(const Ice::PropertiesPtr& properties, const SessionCallbackPtr& callback) :
    _routerHost("127.0.0.1"),
    _secure(true),
    _port(0),
    _timeout(10000),
    _callback(callback)
{
    if(!properties)
    {
        throw Ice::InitializationException(
            __FILE__, __LINE__, "Attempt to create a SessionFactoryHelper with a null Properties argument");
    }
    _identity.name = "router";
    _identity.category = "Glacier2";
    _initData.properties = properties;
    setDefaultProperties();
}

void
Glacier2::SessionFactoryHelper::setRouterIdentity(const Ice::Identity& identity)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _identity = identity;
}

Ice::Identity
Glacier2::SessionFactoryHelper::getRouterIdentity() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _identity;
}

void
Glacier2::SessionFactoryHelper::setRouterHost(const string& hostname)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _routerHost = hostname;
}

string
Glacier2::SessionFactoryHelper::getRouterHost() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _routerHost;
}

void
Glacier2::SessionFactoryHelper::setSecure(bool secure)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _secure = secure;
}

bool
Glacier2::SessionFactoryHelper::getSecure() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _secure;
}

void
Glacier2::SessionFactoryHelper::setTimeout(int timeout)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _timeout = timeout;
}

int
Glacier2::SessionFactoryHelper::getTimeout() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _timeout;
}

void
Glacier2::SessionFactoryHelper::setPort(int port)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _port = port;
}

int
Glacier2::SessionFactoryHelper::getPort() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _port;
}

Ice::InitializationData
Glacier2::SessionFactoryHelper::getInitializationData() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _initData;
}

void
Glacier2::SessionFactoryHelper::setConnectContext(map<string, string> context)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _context = context;
}

Glacier2::SessionHelperPtr
Glacier2::SessionFactoryHelper::connect()
{
    IceUtil::Mutex::Lock sync(_mutex);
    Glacier2::SessionHelperPtr session = new Glacier2::SessionHelper(_callback, createInitData());
    session->connect(_context);
    return session;
}

Glacier2::SessionHelperPtr
Glacier2::SessionFactoryHelper::connect(const string& user,  const string& password)
{
    IceUtil::Mutex::Lock sync(_mutex);
    Glacier2::SessionHelperPtr session = new Glacier2::SessionHelper(_callback, createInitData());
    session->connect(user, password, _context);
    return session;
}

Ice::InitializationData
Glacier2::SessionFactoryHelper::createInitData()
{
    //
    // Clone the initialization data and properties.
    //
    Ice::InitializationData initData = _initData;
    initData.properties = initData.properties->clone();

    if(initData.properties->getProperty("Ice.Default.Router").size() == 0)
    {
        ostringstream os;
        os << "\"";
        
        //
        // TODO replace with identityToString, we cannot use the Communicator::identityToString
        // current implementation because we need to do that before the communicator has been
        // initialized.
        //
        if(!_identity.category.empty())
        {
            os << _identity.category << "/";
        }
        os << _identity.name;
        
        os << "\"";
        os << ":";
        if(_secure)
        {
            os << "ssl -p ";
        }
        else
        {
            os << "tcp -p ";
        }
        
        if(_port != 0)
        {
            os << _port;
        }
        else
        {
            if(_secure)
            {
                os << GLACIER2_SSL_PORT;
            }
            else
            {
                os << GLACIER2_TCP_PORT;
            }
        }

        os << " -h ";
        os << _routerHost;
        if(_timeout > 0)
        {
            os << " -t ";
            os << _timeout;
        }
        initData.properties->setProperty("Ice.Default.Router", os.str());
    }
    return initData;
}

void
Glacier2::SessionFactoryHelper::setDefaultProperties()
{
    assert(_initData.properties);
    _initData.properties->setProperty("Ice.ACM.Client", "0");
    _initData.properties->setProperty("Ice.RetryIntervals", "-1");
}
