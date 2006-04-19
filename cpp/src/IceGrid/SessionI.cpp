// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

SessionReapable::SessionReapable(const SessionIPtr& session, const SessionPrx& proxy) : 
    _session(session),
    _proxy(proxy)
{
}

SessionReapable::~SessionReapable()
{
}

IceUtil::Time
SessionReapable::timestamp() const
{
    return _session->timestamp();
}

void
SessionReapable::destroy()
{
    _proxy->destroy();
}

SessionI::SessionI(const string& userId, const string& prefix, const DatabasePtr& database, int timeout) :
    _userId(userId), 
    _prefix(prefix),
    _timeout(timeout),
    _traceLevels(database->getTraceLevels()),
    _database(database),
    _destroyed(false)
{
    if(_traceLevels && _traceLevels->session > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
	out << _prefix << " session `" << _userId << "' created";
    }
}

SessionI::~SessionI()
{
}

void
SessionI::keepAlive(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    _timestamp = IceUtil::Time::now();

    if(_traceLevels->session > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
	out << _prefix << " session `" << _userId << "' keep alive";
    }
}

int
SessionI::getTimeout(const Ice::Current&) const
{
    return _timeout;
}

QueryPrx
SessionI::getQuery(const Ice::Current& current) const
{
    //
    // TODO: XXX
    //
    return QueryPrx::uncheckedCast(
	current.adapter->getCommunicator()->stringToProxy(_database->getInstanceName() + "/Query")); 
}

Ice::LocatorPrx
SessionI::getLocator(const Ice::Current& current) const
{
    //
    // TODO: XXX
    //
    return Ice::LocatorPrx::uncheckedCast(
	current.adapter->getCommunicator()->stringToProxy(_database->getInstanceName() + "/Locator")); 
}

void
SessionI::allocateObject(const Ice::ObjectPrx& proxy, const Ice::Current&)
{
    //
    // TODO: XXX
    //
}

void
SessionI::releaseObject(const Ice::ObjectPrx& proxy, const Ice::Current&)
{
    //
    // TODO: XXX
    //
}

void
SessionI::setAllocationTimeout(int timeout, const Ice::Current&)
{
    //
    // TODO: XXX
    //
}

void
SessionI::destroy(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }
    _destroyed = true;
    current.adapter->remove(current.id);

    if(_traceLevels && _traceLevels->session > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
	out << _prefix << " session `" << _userId << "' destroyed";
    }
}

IceUtil::Time
SessionI::timestamp() const
{
    Lock sync(*this);
    return _timestamp;
}

ClientSessionI::ClientSessionI(const string& userId, const DatabasePtr& database, int timeout) :
    SessionI(userId, "client", database, timeout)
{
}

ClientSessionManagerI::ClientSessionManagerI(const DatabasePtr& database,
					     const ReapThreadPtr& reaper,
					     int sessionTimeout) :
    _database(database), 
    _reaper(reaper),
    _sessionTimeout(sessionTimeout)
{
}

Glacier2::SessionPrx
ClientSessionManagerI::create(const string& userId, const Glacier2::SessionControlPrx&, const Ice::Current& current)
{
    //
    // We don't add the session to the reaper thread, Glacier2 takes
    // care of reaping the session.
    //
    SessionIPtr session = new ClientSessionI(userId, _database, _sessionTimeout);
    return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
}

SessionPrx
ClientSessionManagerI::createLocalSession(const string& userId, const Ice::Current& current)
{
    SessionIPtr session = new ClientSessionI(userId, _database, _sessionTimeout);
    SessionPrx proxy = SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    _reaper->add(new SessionReapable(session, proxy));
    return proxy;
}

