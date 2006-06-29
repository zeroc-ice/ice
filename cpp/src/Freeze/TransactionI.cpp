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
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Ice;

DbTxn*
Freeze::getTxn(const Freeze::TransactionPtr& tx)
{
    return dynamic_cast<Freeze::TransactionI*>(tx.get())->dbTxn();
}

void
Freeze::TransactionI::commit()
{
    assert(_txn != 0);

    long txnId = 0;
    try
    {
	_connection->closeAllIterators();

	if(_txTrace >= 1)
	{
	    txnId = (_txn->id() & 0x7FFFFFFF) + 0x80000000L;
	}

	_txn->commit(0);

	if(_txTrace >= 1)
	{
	    Trace out(_connection->communicator()->getLogger(), "Freeze.Map");
	    out << "committed transaction " << hex << txnId << dec;
	}
    }
    catch(const ::DbDeadlockException& dx)
    {
	if(_txTrace >= 1)
	{
	    Trace out(_connection->communicator()->getLogger(), "Freeze.Map");
	    out << "failed to commit transaction " << hex << txnId << dec << ": " << dx.what();
	}

	cleanup();
	DeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	if(_txTrace >= 1)
	{
	    Trace out(_connection->communicator()->getLogger(), "Freeze.Map");
	    out << "failed to commit transaction " << hex << txnId << dec << ": " << dx.what();
	}

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

    long txnId = 0;
    try
    {
	_connection->closeAllIterators();

	if(_txTrace >= 1)
	{
	    txnId = (_txn->id() & 0x7FFFFFFF) + 0x80000000L;
	}

	_txn->abort();

	if(_txTrace >= 1)
	{
	    Trace out(_connection->communicator()->getLogger(), "Freeze.Map");
	    out << "rolled back transaction " << hex << txnId << dec;
	}
    }
    catch(const ::DbDeadlockException& dx)
    {
	if(_txTrace >= 1)
	{
	    Trace out(_connection->communicator()->getLogger(), "Freeze.Map");
	    out << "failed to rollback transaction " << hex << txnId << dec << ": " << dx.what();
	}

	cleanup();
	DeadlockException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    catch(const ::DbException& dx)
    {
	if(_txTrace >= 1)
	{
	    Trace out(_connection->communicator()->getLogger(), "Freeze.Map");
	    out << "failed to rollback transaction " << hex << txnId << dec << ": " << dx.what();
	}

	cleanup();
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    cleanup();
}
    
Freeze::TransactionI::TransactionI(ConnectionI* connection) :
    _connection(connection),
    _txTrace(connection->txTrace()),
    _txn(0)
{
    try
    {
	_connection->dbEnv()->getEnv()->txn_begin(0, &_txn, 0);

	if(_txTrace >= 1)
	{
	    long txnId = (_txn->id() & 0x7FFFFFFF) + 0x80000000L;
	    Trace out(_connection->communicator()->getLogger(), "Freeze.Map");
	    out << "started transaction " << hex << txnId << dec;
	}
    }
    catch(const ::DbException& dx)
    {
	if(_txTrace >= 1)
	{
	    Trace out(_connection->communicator()->getLogger(), "Freeze.Map");
	    out << "failed to start transaction: " << dx.what();
	}

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
