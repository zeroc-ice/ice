// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/RouterI.h>
#include <Glacier2/Session.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::RouterI::RouterI(const ObjectAdapterPtr& clientAdapter, const ObjectAdapterPtr& serverAdapter,
			   const ConnectionPtr& connection, const string& userId, const SessionPrx& session) :
    _communicator(clientAdapter->getCommunicator()),
    _routingTable(new RoutingTable(_communicator)),
    _clientProxy(clientAdapter->createProxy(stringToIdentity("dummy"))),
    _connection(connection),
    _userId(userId),
    _session(session),
    _timestamp(IceUtil::Time::now()),
    _destroy(false)
{
    string allow = _communicator->getProperties()->getProperty("Glacier2.AllowCategories");
    StringSeq allowCategories;
    
    const string ws = " \t";
    string::size_type current = allow.find_first_not_of(ws, 0);
    while(current != string::npos)
    {
	string::size_type pos = allow.find_first_of(ws, current);
	string::size_type len = (pos == string::npos) ? string::npos : pos - current;
	string category = allow.substr(current, len);
	allowCategories.push_back(category);
	current = allow.find_first_not_of(ws, pos);
    }

    int addUserMode = _communicator->getProperties()->getPropertyAsInt("Glacier2.AddUserToAllowCategories");
    if(addUserMode == 1)
    {
	allowCategories.push_back(_userId); // Add user id to allowed categories.
    }
    else if(addUserMode == 2)
    {
	allowCategories.push_back('_' + _userId); // Add user id with prepended underscore to allowed categories.
    }

    sort(allowCategories.begin(), allowCategories.end()); // Must be sorted.
    allowCategories.erase(unique(allowCategories.begin(), allowCategories.end()), allowCategories.end());

    const_cast<ClientBlobjectPtr&>(_clientBlobject) = new ClientBlobject(_communicator, _routingTable,
									 allowCategories);

    if(serverAdapter)
    {
	ObjectPrx& serverProxy = const_cast<ObjectPrx&>(_serverProxy);
	Identity ident;
	ident.name = "dummy";
	ident.category.resize(20);
	for(string::iterator p = ident.category.begin(); p != ident.category.end(); ++p)
	{
	    *p = static_cast<char>(33 + rand() % (127-33)); // We use ASCII 33-126 (from ! to ~, w/o space).
	}
	serverProxy = serverAdapter->createProxy(ident);
	
	ServerBlobjectPtr& serverBlobject = const_cast<ServerBlobjectPtr&>(_serverBlobject);
	serverBlobject = new ServerBlobject(_communicator, _connection);
    }
}

Glacier2::RouterI::~RouterI()
{
    IceUtil::Mutex::Lock lock(*this);

    assert(_destroy);
}

void
Glacier2::RouterI::destroy()
{
    IceUtil::Mutex::Lock lock(*this);

    assert(!_destroy);
    _destroy = true;

    _connection->close(true);

    _clientBlobject->destroy();
    
    if(_serverBlobject)
    {
	_serverBlobject->destroy();
    }

    if(_session)
    {
	//
	// This can raise an exception, therefore it must be the last
	// statement in this destroy() function.
	//
	_session->destroy();
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
    IceUtil::Mutex::Lock lock(*this);

    assert(!_destroy);

    _timestamp = IceUtil::Time::now();

    _routingTable->add(proxy);
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

void
Glacier2::RouterI::destroySession(const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

ClientBlobjectPtr
Glacier2::RouterI::getClientBlobject() const
{
    IceUtil::Mutex::Lock lock(*this);

    assert(!_destroy);

    _timestamp = IceUtil::Time::now();

    return _clientBlobject;
}

ServerBlobjectPtr
Glacier2::RouterI::getServerBlobject() const
{
    IceUtil::Mutex::Lock lock(*this);

    assert(!_destroy);

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

    out << "user-id = " << _userId << '\n';
    if(_serverProxy)
    {
	out << "category = " << _serverProxy->ice_getIdentity().category << '\n';
    }
    out << _connection->toString();

    return out.str();
}
