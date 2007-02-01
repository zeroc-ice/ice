// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>

#include <Glacier2/FilterManager.h>
#include <Glacier2/RoutingTable.h>
#include <Glacier2/RouterI.h>
#include <Glacier2/Session.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::RouterI::RouterI(const ObjectAdapterPtr& clientAdapter, const ObjectAdapterPtr& serverAdapter,
                           const ConnectionPtr& connection, const string& userId, const SessionPrx& session,
                           const Identity& controlId, const FilterManagerPtr& filters,
                           const Ice::Context& sslContext) :
    _communicator(clientAdapter->getCommunicator()),
    _clientBlobject(new ClientBlobject(_communicator, filters, sslContext)),
    _serverAdapter(serverAdapter),
    _connection(connection),
    _userId(userId),
    _session(session),
    _controlId(controlId),
    _sslContext(sslContext),
    _timestamp(IceUtil::Time::now())
{
    //
    // If Glacier2 will be used with pre 3.2 clients, then the client proxy must be set.
    // Otherwise getClientProxy just needs to return a nil proxy.
    //
    if(_communicator->getProperties()->getPropertyAsInt("Glacier2.ReturnClientProxy") > 0)
    {
        const_cast<Ice::ObjectPrx&>(_clientProxy) = 
            clientAdapter->createProxy(_communicator->stringToIdentity("dummy"));
    }

    if(serverAdapter)
    {
        ObjectPrx& serverProxy = const_cast<ObjectPrx&>(_serverProxy);
        Identity ident;
        ident.name = "dummy";
        ident.category.resize(20);
        char buf[20];
        IceUtil::generateRandom(buf, static_cast<int>(sizeof(buf)));
        for(unsigned int i = 0; i < sizeof(buf); ++i)
        {
            const unsigned char c = static_cast<unsigned char>(buf[i]); // A value between 0-255
            ident.category[i] = 33 + c % (127-33); // We use ASCII 33-126 (from ! to ~, w/o space).
        }
        serverProxy = serverAdapter->createProxy(ident);

        ServerBlobjectPtr& serverBlobject = const_cast<ServerBlobjectPtr&>(_serverBlobject);
        serverBlobject = new ServerBlobject(_communicator, _connection);
    }
}

Glacier2::RouterI::~RouterI()
{
}

void
Glacier2::RouterI::destroy()
{
    _connection->close(true);

    _clientBlobject->destroy();
    
    if(_serverBlobject)
    {
        _serverBlobject->destroy();
    }

    if(_session)
    {
        if(_serverAdapter)
        {
            try
            {
                //
                // Remove the session control object.
                //
                _serverAdapter->remove(_controlId);
            }
            catch(const NotRegisteredException&)
            {
            }
            catch(const ObjectAdapterDeactivatedException&)
            {
                //
                // Expected if the router has been shutdown.
                //
            }
        }

        //
        // This can raise an exception, therefore it must be the last
        // statement in this destroy() function.
        //
        if(_sslContext.size() > 0)
        {
            _session->destroy(_sslContext);
        }
        else
        {
            _session->destroy();
        }
    }
}

ObjectPrx
Glacier2::RouterI::getClientProxy(const Current&) const
{
    // No mutex lock necessary, _clientProxy is immutable and is never destroyed.
    return _clientProxy;
}

ObjectPrx
Glacier2::RouterI::getServerProxy(const Current&) const
{
    // No mutex lock necessary, _serverProxy is immutable and is never destroyed.
    return _serverProxy;
}

void
Glacier2::RouterI::addProxy(const ObjectPrx& proxy, const Current& current)
{
    ObjectProxySeq proxies;
    proxies.push_back(proxy);
    addProxies(proxies, current);
}

ObjectProxySeq
Glacier2::RouterI::addProxies(const ObjectProxySeq& proxies, const Current& current)
{
    IceUtil::Mutex::Lock lock(*this);

    _timestamp = IceUtil::Time::now();

    return _clientBlobject->add(proxies, current);
}

string
Glacier2::RouterI::getCategoryForClient(const Ice::Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

SessionPrx
Glacier2::RouterI::createSession(const std::string&, const std::string&, const Current&)
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

SessionPrx
Glacier2::RouterI::createSessionFromSecureConnection(const Current&)
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

void
Glacier2::RouterI::destroySession(const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

Ice::Long
Glacier2::RouterI::getSessionTimeout(const Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

ClientBlobjectPtr
Glacier2::RouterI::getClientBlobject() const
{
    IceUtil::Mutex::Lock lock(*this);

    _timestamp = IceUtil::Time::now();

    return _clientBlobject;
}

ServerBlobjectPtr
Glacier2::RouterI::getServerBlobject() const
{
    //
    // We do not update the timestamp for callbacks from the
    // server. We only update the timestamp for client activity.
    //

    return _serverBlobject;
}

SessionPrx
Glacier2::RouterI::getSession() const
{
    return _session; // No mutex lock necessary, _session is immutable.
}

IceUtil::Time
Glacier2::RouterI::getTimestamp() const
{
    IceUtil::Mutex::TryLock lock(*this);

    if(lock.acquired())
    {
        return _timestamp;
    }
    else
    {
        return IceUtil::Time::now();
    }
}

string
Glacier2::RouterI::toString() const
{
    ostringstream out;

    out << "id = " << _userId << '\n';
    if(_serverProxy)
    {
        out << "category = " << _serverProxy->ice_getIdentity().category << '\n';
    }
    out << _connection->toString();

    return out.str();
}
