// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    if(_txn == 0)
    {
        throw DatabaseException(__FILE__, __LINE__, "inactive transaction");
    }
    
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
            Trace out(_communicator->getLogger(), "Freeze.Transaction");
            out << "committed transaction " << hex << txnId << dec;
        }
    }
    catch(const ::DbDeadlockException& dx)
    {
        if(_txTrace >= 1)
        {
            Trace out(_communicator->getLogger(), "Freeze.Transaction");
            out << "failed to commit transaction " << hex << txnId << dec << ": " << dx.what();
        }

        postCompletion(false, true);
        // After postCompletion is called the transaction may be
        // dead. Beware!
        DeadlockException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
    catch(const ::DbException& dx)
    {
        if(_txTrace >= 1)
        {
            Trace out(_communicator->getLogger(), "Freeze.Transaction");
            out << "failed to commit transaction " << hex << txnId << dec << ": " << dx.what();
        }

        postCompletion(false, false);
        // After postCompletion is called the transaction may be
        // dead. Beware!
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
    postCompletion(true, false);
    // After postCompletion is called the transaction may be
    // dead. Beware!
}

void
Freeze::TransactionI::rollback()
{
    if(_txn != 0)
    {
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
                Trace out(_communicator->getLogger(), "Freeze.Transaction");
                out << "rolled back transaction " << hex << txnId << dec;
            }
        }
        catch(const ::DbDeadlockException& dx)
        {
            if(_txTrace >= 1)
            {
                Trace out(_communicator->getLogger(), "Freeze.Transaction");
                out << "failed to rollback transaction " << hex << txnId << dec << ": " << dx.what();
            }
            
            postCompletion(false, true);
            // After postCompletion is called the transaction may be
            // dead. Beware!
            throw DeadlockException(__FILE__, __LINE__, dx.what());
        }
        catch(const ::DbException& dx)
        {
            if(_txTrace >= 1)
            {
                Trace out(_communicator->getLogger(), "Freeze.Transaction");
                out << "failed to rollback transaction " << hex << txnId << dec << ": " << dx.what();
            }
            
            postCompletion(false, false);
            // After postCompletion is called the transaction may be
            // dead. Beware!
            throw DatabaseException(__FILE__, __LINE__, dx.what());
        }
        postCompletion(true, false);
        // After postCompletion is called the transaction may be
        // dead. Beware!
    }
}

Freeze::ConnectionPtr
Freeze::TransactionI::getConnection() const
{
    return _connection;
}

//
// External refcount operations, from code holding a Transaction[I]Ptr
//
void
Freeze::TransactionI::__decRef()
{
    // If dropping the second to last reference and there is still a
    // transaction then this means the last reference is held by the
    // connection. In this case we must rollback the transaction.
    bool rb = false;
    if(__getRef() == 2 && _txn)
    {
        rb = true;
    }
    Shared::__decRef();
    if(rb)
    {
        rollback();
        // After this the transaction is dead.
    }
}

void
Freeze::TransactionI::setPostCompletionCallback(const Freeze::PostCompletionCallbackPtr& cb)
{
    _postCompletionCallback = cb;
}
    
Freeze::TransactionI::TransactionI(const ConnectionIPtr& connection) :
    _communicator(connection->communicator()),
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
            Trace out(_communicator->getLogger(), "Freeze.Transaction");
            out << "started transaction " << hex << txnId << dec;
        }
    }
    catch(const ::DbException& dx)
    {
        if(_txTrace >= 1)
        {
            Trace out(_communicator->getLogger(), "Freeze.Transaction");
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
        try
        {
            rollback();
        }
        catch(const IceUtil::Exception& e)
        {
            Error error(_communicator->getLogger());
            error << "transaction rollback raised :" << e;
        }
    }
}

void
Freeze::TransactionI::postCompletion(bool committed, bool deadlock)
{
    // The order of assignment in this method is very important as
    // calling both the post completion callback and
    // Connection::clearTransaction may alter the transaction
    // reference count which checks _txn.
    _txn = 0;

    if(_postCompletionCallback != 0)
    {
        _postCompletionCallback->postCompletion(committed, deadlock);
    }

    // Its necessary here to copy the connection before calling
    // clearTransaction because this may release the last reference. This specifically
    // occurs in the following scenario:
    //
    // TransactionalEvictorContext holds the _tx. It calls
    // _tx->commit(). This comes into this method, and calls
    // _postCompletionCallback. This causes the context to drop the
    // _tx reference (reference count is now 1). The
    // connection->clearTransaction() is then called which drops its
    // reference causing the transaction to be deleted.
    //
    ConnectionIPtr con = _connection;
    _connection = 0; // Drop the connection
    con->clearTransaction();
    // At this point the transaction may be dead.
}
