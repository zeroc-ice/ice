// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
    
    if(_dbEnv != 0)
    {
	_dbEnv = 0;
	_dbEnvHolder = 0;
    }
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
				 const string& envName) :
    _communicator(communicator),
    _dbEnvHolder(SharedDbEnv::get(communicator, envName)),
    _envName(envName),
    _trace(communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Map"))
{
    _dbEnv = _dbEnvHolder.get();
}

Freeze::ConnectionI::ConnectionI(const CommunicatorPtr& communicator,
				 const string& envName,
				 DbEnv& dbEnv) :
    _communicator(communicator),
    _dbEnv(&dbEnv),
    _envName(envName),
    _trace(communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Map"))
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
    
    return new ConnectionI(communicator, envName);
}

Freeze::ConnectionPtr 
Freeze::createConnection(const CommunicatorPtr& communicator,
			 const string& envName,
			 DbEnv& dbEnv)
{
    return new ConnectionI(communicator, envName, dbEnv);
}

void
Freeze::TransactionAlreadyInProgressException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntransaction already in progress";
}
