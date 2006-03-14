// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/TransactionI.h>
#include <Freeze/ConnectionI.h>
#include <Freeze/Exception.h>
#include <Freeze/Initialize.h>


DbTxn*
Freeze::getTxn(const Freeze::TransactionPtr& tx)
{
    return dynamic_cast<Freeze::TransactionI*>(tx.get())->dbTxn();
}

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
	_connection->dbEnv()->getEnv()->txn_begin(0, &_txn, 0);
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
