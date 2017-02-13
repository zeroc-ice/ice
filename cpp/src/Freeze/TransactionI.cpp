// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    rollbackInternal(false);
}

Freeze::ConnectionPtr
Freeze::TransactionI::getConnection() const
{
    if(_txn)
    {
        return _connection;
    }
    return 0;
}

void
Freeze::TransactionI::rollbackInternal(bool warning)
{
    if(_txn != 0)
    {
        long txnId = 0;
        try
        {
            _connection->closeAllIterators();
            
            if(_txTrace >= 1 || (warning && _warnRollback))
            {
                txnId = (_txn->id() & 0x7FFFFFFF) + 0x80000000L;
                if(warning && _warnRollback)
                {
                    Warning warn(_communicator->getLogger());
                    warn << "Freeze.Transaction: rolled back transaction " << hex << txnId << dec
                         << " due to destruction.\nApplication code should explicitly call rollback or commit.";
                }
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
            
            DeadlockException deadlockException(__FILE__, __LINE__, dx.what(), this);

            postCompletion(false, true);
            // After postCompletion is called the transaction may be
            // dead. Beware!
            throw deadlockException;
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

void
Freeze::TransactionI::__incRef()
{
    IceUtil::Mutex::Lock sync(_refCountMutex->mutex);
    _refCount++;
}


void
Freeze::TransactionI::__decRef()
{
    IceUtil::Mutex::Lock sync(_refCountMutex->mutex);
    if(--_refCount == 0)
    {
        sync.release();
        delete this;
    }
    else if(_txn != 0 && _refCount == 1 && _connection->__getRefNoSync() == 1)
    {
        sync.release();
        rollbackInternal(true);
    }
}

int
Freeze::TransactionI::__getRef() const
{
    IceUtil::Mutex::Lock sync(_refCountMutex->mutex);
    return _refCount;
}

int
Freeze::TransactionI::__getRefNoSync() const
{
    return _refCount;
}

void
Freeze::TransactionI::setPostCompletionCallback(const Freeze::PostCompletionCallbackPtr& cb)
{
    _postCompletionCallback = cb;
}
    
//
// The constructor takes a ConnectionI* instead of a ConnectionIPtr
// because we have to ensure there is no call to __decRef while the
// transaction or the connection are not assigned to a Ptr in
// user-code.
//
Freeze::TransactionI::TransactionI(ConnectionI* connection) :
    _communicator(connection->communicator()),
    _connection(connection),
    _txTrace(connection->txTrace()),
    _warnRollback(_communicator->getProperties()->getPropertyAsIntWithDefault("Freeze.Warn.Rollback", 1)),
    _txn(0),
    _refCountMutex(connection->_refCountMutex),
    _refCount(0)
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
    assert(_txn == 0);
}

void
Freeze::TransactionI::postCompletion(bool committed, bool deadlock)
{
    // The order of assignment in this method is very important as
    // calling both the post completion callback and
    // Connection::clearTransaction may alter the transaction
    // reference count which checks _txn.

    {
        //
        // We synchronize here as _txn is checked (read) in the refcounting code
        //
        IceUtil::Mutex::Lock sync(_refCountMutex->mutex);
        _txn = 0;
    }

    if(_postCompletionCallback != 0)
    {
        PostCompletionCallbackPtr cb = _postCompletionCallback;
        _postCompletionCallback = 0;
        
        cb->postCompletion(committed, deadlock, _connection->dbEnv());
    }

    ConnectionIPtr connection = _connection;
    _connection = 0;

    connection->clearTransaction(); // may release the last _refCount
}
