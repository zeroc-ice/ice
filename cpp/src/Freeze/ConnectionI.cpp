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
	catch(const Freeze::DBException&)
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
    
Ice::CommunicatorPtr
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

Freeze::ConnectionI::ConnectionI(const Ice::CommunicatorPtr& communicator, 
				 const std::string& envName) :
    _communicator(communicator),
    _dbEnvHolder(SharedDbEnv::get(communicator, envName)),
    _envName(envName),
    _trace(communicator->getProperties()->getPropertyAsInt("Freeze.Trace.DB"))
{
    _dbEnv = _dbEnvHolder.get();
}

Freeze::ConnectionI::ConnectionI(const Ice::CommunicatorPtr& communicator, 
				 DbEnv& dbEnv) :
    _communicator(communicator),
    _dbEnv(&dbEnv),
    _envName("External"),
    _trace(communicator->getProperties()->getPropertyAsInt("Freeze.Trace.DB"))
{
}

void
Freeze::ConnectionI::closeAllIterators()
{
    for(list<DBMapHelperI*>::iterator p = _mapList.begin(); p != _mapList.end();
	++p)
    {
	(*p)->closeAllIterators();
    }
}

void
Freeze::ConnectionI::registerMap(DBMapHelperI* map)
{
    _mapList.push_back(map);
}

void
Freeze::ConnectionI::unregisterMap(DBMapHelperI* map)
{
    _mapList.remove(map);
}

Freeze::ConnectionPtr 
Freeze::createConnection(const CommunicatorPtr& communicator,
			 const string& envName)
{
    
    return new ConnectionI(communicator, envName);
}

Freeze::ConnectionPtr 
Freeze::createConnection(const CommunicatorPtr& communicator,
			 DbEnv& dbEnv)
{
    return new ConnectionI(communicator, dbEnv);
}

void
Freeze::TransactionAlreadyInProgressException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntransaction already in progess";
}
