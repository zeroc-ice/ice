// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/ConnectionI.h>
#include <Freeze/MapI.h>
#include <Freeze/Initialize.h>

using namespace Ice;
using namespace Freeze;
using namespace std;


Freeze::TransactionPtr
Freeze::ConnectionI::beginTransaction()
{
    if(_transaction != 0)
    {
	throw TransactionAlreadyInProgressException(__FILE__, __LINE__);
    }
    closeAllIterators();
    _transaction = new TransactionI(this);
    return _transaction;
}

Freeze::TransactionPtr
Freeze::ConnectionI::currentTransaction() const
{
    return _transaction;
}

void
Freeze::ConnectionI::close()
{
    if(_transaction != 0)
    {
	try
	{
	    _transaction->rollback();
	}
	catch(const  DatabaseException&)
	{
	    //
	    // Ignored
	    //
	}
    }

    while(!_mapList.empty())
    {
	(*_mapList.begin())->close();
    }
    
    _dbEnv = 0;
 
}
    
CommunicatorPtr
Freeze::ConnectionI::getCommunicator() const
{
    return _communicator;
}

string
Freeze::ConnectionI::getName() const
{
    return _envName;
}


Freeze::ConnectionI::~ConnectionI()
{
    close();
}

Freeze::ConnectionI::ConnectionI(const CommunicatorPtr& communicator, 
				 const string& envName, DbEnv* dbEnv) :
    _communicator(communicator),
    _dbEnv(SharedDbEnv::get(communicator, envName, dbEnv)),
    _envName(envName),
    _trace(communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Map")),
    _deadlockWarning(communicator->getProperties()->getPropertyAsInt("Freeze.Warn.Deadlocks") != 0)
{
}

void
Freeze::ConnectionI::closeAllIterators()
{
    for(list<MapHelperI*>::iterator p = _mapList.begin(); p != _mapList.end();
	++p)
    {
	(*p)->closeAllIterators();
    }
}

void
Freeze::ConnectionI::registerMap(MapHelperI* m)
{
    _mapList.push_back(m);
}

void
Freeze::ConnectionI::unregisterMap(MapHelperI* m)
{
    _mapList.remove(m);
}

Freeze::ConnectionPtr 
Freeze::createConnection(const CommunicatorPtr& communicator,
			 const string& envName)
{
    
    return new ConnectionI(communicator, envName, 0);
}

Freeze::ConnectionPtr 
Freeze::createConnection(const CommunicatorPtr& communicator,
			 const string& envName,
			 DbEnv& dbEnv)
{
    return new ConnectionI(communicator, envName, &dbEnv);
}

void
Freeze::TransactionAlreadyInProgressException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntransaction already in progress";
}
