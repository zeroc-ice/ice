// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    return beginTransactionI();
}

Freeze::TransactionIPtr
Freeze::ConnectionI::beginTransactionI()
{
    if(_transaction)
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
Freeze::ConnectionI::removeMapIndex(const string& mapName, const string& indexName)
{
    if(_dbEnv == 0)
    {
        throw DatabaseException(__FILE__, __LINE__, "Closed connection");
    }

    string filename = mapName + "." + indexName;

    DbTxn* txn = dbTxn();
    try
    {
        _dbEnv->getEnv()->dbremove(txn, filename.c_str(), 0, txn != 0 ? 0 : DB_AUTO_COMMIT);
    }
    catch(const DbDeadlockException& dx)
    {
        throw DeadlockException(__FILE__, __LINE__, dx.what(), _transaction);
    }
    catch(const DbException& dx)
    {
        if(dx.get_errno() == ENOENT)
        {
            throw IndexNotFoundException(__FILE__, __LINE__, mapName, indexName);
        }
        else
        {
            throw DatabaseException(__FILE__, __LINE__, dx.what());
        }
    }
}


void
Freeze::ConnectionI::close()
{
    if(_transaction)
    {
        _transaction->rollbackInternal(true);
        assert(_transaction == 0);
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

EncodingVersion
Freeze::ConnectionI::getEncoding() const
{
    return _encoding;
}

string
Freeze::ConnectionI::getName() const
{
    return _envName;
}

void
Freeze::ConnectionI::__incRef()
{
    IceUtil::Mutex::Lock sync(_refCountMutex->mutex);
    _refCount++;
}


void
Freeze::ConnectionI::__decRef()
{
    IceUtil::Mutex::Lock sync(_refCountMutex->mutex);
    if(--_refCount == 0)
    {
        sync.release();
        delete this;
    }
    else if(_refCount == 1 && _transaction != 0 && _transaction->dbTxn() != 0 && _transaction->__getRefNoSync() == 1)
    {
        sync.release();
        if(_transaction)
        {
            //
            // This makes the transaction release the last refcount on the connection
            //
            _transaction->rollbackInternal(true);
        }
    }
}

int
Freeze::ConnectionI::__getRef() const
{
    IceUtil::Mutex::Lock sync(_refCountMutex->mutex);
    return _refCount;
}

int
Freeze::ConnectionI::__getRefNoSync() const
{
    return _refCount;
}


Freeze::ConnectionI::~ConnectionI()
{
    try
    {
        close();
    }
    catch(const DatabaseException& ex)
    {
        Ice::Error error(_communicator->getLogger());
        error << "Freeze: closing connection Freeze.DbEnv: " << _envName << " raised DatabaseException: " << ex.what();
    }
}

Freeze::ConnectionI::ConnectionI(const SharedDbEnvPtr& dbEnv) :
    _communicator(dbEnv->getCommunicator()),
    _encoding(dbEnv->getEncoding()),
    _dbEnv(dbEnv),
    _envName(dbEnv->getEnvName()),
    _trace(_communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Map")),
    _txTrace(_communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Transaction")),
    _deadlockWarning(_communicator->getProperties()->getPropertyAsInt("Freeze.Warn.Deadlocks") > 0),
    _refCountMutex(new SharedMutex),
    _refCount(0)
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
Freeze::createConnection(const CommunicatorPtr& communicator, const string& envName)
{

    return new ConnectionI(SharedDbEnv::get(communicator, envName, 0));
}

Freeze::ConnectionPtr
Freeze::createConnection(const CommunicatorPtr& communicator, const string& envName, DbEnv& dbEnv)
{
    return new ConnectionI(SharedDbEnv::get(communicator, envName, &dbEnv));
}
