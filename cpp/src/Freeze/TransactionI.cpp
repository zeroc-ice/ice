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

#include <Freeze/TransactionI.h>
#include <Freeze/ConnectionI.h>
#include <Freeze/Exception.h>


void
Freeze::TransactionI::commit()
{
    assert(_txn != 0);
    try
    {
	_connection->closeAllIterators();
	_txn->commit(0);
    }
    catch(const ::DbDeadlockException& dx)
    {
	cleanup();
	DeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	cleanup();
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    cleanup();
}

void
Freeze::TransactionI::rollback()
{
    assert(_txn != 0);

    try
    {
	_connection->closeAllIterators();
	_txn->abort();
    }
    catch(const ::DbDeadlockException& dx)
    {
	cleanup();
	DeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	cleanup();
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    cleanup();
}
    
Freeze::TransactionI::TransactionI(ConnectionI* connection) :
    _connection(connection),
    _txn(0)
{
    try
    {
	_connection->dbEnv()->txn_begin(0, &_txn, 0);
    }
    catch(const ::DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}
    
Freeze::TransactionI::~TransactionI()
{
    if(_txn != 0)
    {
	rollback();
    }
}

void
Freeze::TransactionI::cleanup()
{
    _connection->clearTransaction();
    _connection = 0;
    _txn = 0;
}
