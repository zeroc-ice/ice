// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>

#include <Glacier2/RouterI.h>
#include <Glacier2/Session.h>
#include <Glacier2/FilterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

//
// Parse a space delimited string into a sequence of strings.
//
static void
stringToSeq(const string& str, vector<string>& seq)
{
    string const ws = " \t";
    string::size_type current = str.find_first_not_of(ws, 0);
    while(current != string::npos)
    {
	string::size_type pos = str.find_first_of(ws, current);
	string::size_type len = (pos == string::npos) ? string::npos : pos - current;
	seq.push_back(str.substr(current, len));
	current = str.find_first_not_of(ws, pos);
    }
}

Glacier2::RouterI::RouterI(const ObjectAdapterPtr& clientAdapter, const ObjectAdapterPtr& serverAdapter,
			   const ObjectAdapterPtr& adminAdapter, const ConnectionPtr& connection, 
			   const string& userId, bool allowAddUserMode, const SessionPrx& session,
			   const Identity& controlId) :
    _communicator(clientAdapter->getCommunicator()),
    _routingTable(new RoutingTable(_communicator)),
    _clientProxy(clientAdapter->createProxy(_communicator->stringToIdentity("dummy"))),
    _adminAdapter(adminAdapter),
    _connection(connection),
    _userId(userId),
    _session(session),
    _controlId(controlId),
    _timestamp(IceUtil::Time::now())
{
    PropertiesPtr props = _communicator->getProperties();
    //
    // DEPRECATED PROPERTY: Glacier2.AllowCategories is to be deprecated
    // and superseded by Glacier2.Filter.Categories.Allow.
    //
    string allow = props->getProperty("Glacier2.AllowCategories");
    if(allow.empty())
    {
	allow = props->getProperty("Glacier2.Filter.Category.Accept");
    }

    string reject = props->getProperty("Glacier2.Filter.Category.Reject");
    bool acceptOverride = props->getPropertyAsIntWithDefault("Glacier2.Filter.Category.AcceptOverride", 0) == 1;

    vector<string> allowSeq;
    vector<string> rejectSeq;
    stringToSeq(allow, allowSeq);
    stringToSeq(reject, rejectSeq);

    int addUserMode = props->getPropertyAsInt("Glacier2.AddUserToAllowCategories");
    if(addUserMode == 0)
    {
	addUserMode = props->getPropertyAsInt("Glacier2.Filter.Category.AddUser");
    }
   
    if(addUserMode > 0 && !_userId.empty())
    {
	if(addUserMode == 1)
	{
	    allowSeq.push_back(_userId); // Add user id to allowed categories.
	}
	else if(addUserMode == 2)
	{
	    allowSeq.push_back('_' + _userId); // Add user id with prepended underscore to allowed categories.
	}
    }	
    StringFilterIPtr categoryFilter = new StringFilterI(allowSeq, rejectSeq, acceptOverride);

    //
    // TODO: refactor initialization of filters.
    //
    allow = props->getProperty("Glacier2.Filter.AdapterId.Accept");
    reject = props->getProperty("Glacier2.Filter.AdapterId.Reject");
    acceptOverride = props->getPropertyAsIntWithDefault("Glacier2.Filter.AdapterId.AcceptOverride", 0) == 1;
    stringToSeq(allow, allowSeq);
    stringToSeq(reject, rejectSeq);
    StringFilterIPtr adapterIdFilter = new StringFilterI(allowSeq, rejectSeq, acceptOverride);

    //
    // TODO: Object id's from configurations?
    // 
    IdentitySeq allowIdSeq;
    IdentitySeq rejectIdSeq;
    IdentityFilterIPtr objectIdFilter = new IdentityFilterI(allowIdSeq, rejectIdSeq, false);
    
    if(adminAdapter)
    {
        const_cast<StringFilterPrx&>(_categoryFilter) =
	    StringFilterPrx::uncheckedCast(_adminAdapter->addWithUUID(categoryFilter));
        const_cast<StringFilterPrx&>(_adapterIdFilter) =
	    StringFilterPrx::uncheckedCast(_adminAdapter->addWithUUID(adapterIdFilter));
        const_cast<IdFilterPrx&>(_objectIdFilter) =
	    IdFilterPrx::uncheckedCast(_adminAdapter->addWithUUID(objectIdFilter));
    }

    const_cast<ClientBlobjectPtr&>(_clientBlobject) = new ClientBlobject(_communicator, _routingTable,
									 categoryFilter, adapterIdFilter,
									 objectIdFilter);

    if(serverAdapter)
    {
	ObjectPrx& serverProxy = const_cast<ObjectPrx&>(_serverProxy);
	Identity ident;
	ident.name = "dummy";
	ident.category.resize(20);
	char buf[20];
	IceUtil::generateRandom(buf, sizeof(buf));
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
        if(_adminAdapter)
	{
	    try
	    {
	        //
	        // Remove the session control object.
	        //
	        _adminAdapter->remove(_controlId);

		//
		// Remove filter objects.
		//
		_adminAdapter->remove(_objectIdFilter->ice_getIdentity());
		_adminAdapter->remove(_adapterIdFilter->ice_getIdentity());
		_adminAdapter->remove(_categoryFilter->ice_getIdentity());
	    }
	    catch(const NotRegisteredException&)
	    {
	    }
	}

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
    ObjectProxySeq proxies;
    proxies.push_back(proxy);
    addProxies(proxies, current);
}

ObjectProxySeq
Glacier2::RouterI::addProxies(const ObjectProxySeq& proxies, const Current& current)
{
    IceUtil::Mutex::Lock lock(*this);

    _timestamp = IceUtil::Time::now();

    return _routingTable->add(proxies, current);
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

StringFilterPrx
Glacier2::RouterI::getCategoryFilter() const
{
    assert(_adminAdapter);
    return _categoryFilter;
}

StringFilterPrx
Glacier2::RouterI::getAdapterIdFilter() const
{
    assert(_adminAdapter);
    return _adapterIdFilter;
}

IdFilterPrx
Glacier2::RouterI::getObjectIdFilter() const
{
    assert(_adminAdapter);
    return _objectIdFilter;
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
