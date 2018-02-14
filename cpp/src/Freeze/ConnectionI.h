// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_CONNECTIONI_H
#define FREEZE_CONNECTIONI_H

#include <Freeze/Connection.h>
#include <Freeze/Initialize.h>
#include <Freeze/TransactionI.h>
#include <Freeze/SharedDbEnv.h>
#include <list>

namespace Freeze
{

class MapHelperI;

//
// A mutex shared by a connection and all its transactions
// (for refcounting thread-safety)
//
struct SharedMutex : public IceUtil::Shared
{
    IceUtil::Mutex mutex;
};
typedef IceUtil::Handle<SharedMutex> SharedMutexPtr;


class ConnectionI : public Connection
{
public:

    virtual TransactionPtr beginTransaction();

    virtual TransactionPtr currentTransaction() const;

    virtual void removeMapIndex(const std::string&, const std::string&);

    virtual void close();
    
    virtual Ice::CommunicatorPtr getCommunicator() const;

    virtual Ice::EncodingVersion getEncoding() const;

    virtual std::string getName() const;

    //
    // Custom refcounting implementation
    //
    virtual void __incRef();
    virtual void __decRef();
    virtual int __getRef() const;
 
    virtual ~ConnectionI();

    ConnectionI(const SharedDbEnvPtr&);

    TransactionIPtr beginTransactionI();

    void closeAllIterators();

    void registerMap(MapHelperI*);

    void unregisterMap(MapHelperI*);

    void clearTransaction();

    DbTxn* dbTxn() const;

    const SharedDbEnvPtr& dbEnv() const;
       
    const Ice::CommunicatorPtr& communicator() const;
    const Ice::EncodingVersion& encoding() const;

    const std::string&  envName() const;
    
    Ice::Int trace() const;

    Ice::Int txTrace() const;

    bool deadlockWarning() const;

private:

    friend class TransactionI;

    int __getRefNoSync() const;

    const Ice::CommunicatorPtr _communicator;
    const Ice::EncodingVersion _encoding;
    SharedDbEnvPtr _dbEnv;
    const std::string _envName;
    TransactionIPtr _transaction;
    std::list<MapHelperI*> _mapList;
    const Ice::Int _trace;
    const Ice::Int _txTrace;
    const bool _deadlockWarning;
    SharedMutexPtr _refCountMutex;
    int _refCount;
};  
typedef IceUtil::Handle<ConnectionI> ConnectionIPtr;

inline void
ConnectionI::clearTransaction()
{
    _transaction = 0;
}

inline DbTxn*
ConnectionI::dbTxn() const
{
    if(!_transaction)
    {
        return 0;
    }
    else
    {
        return _transaction->dbTxn();
    }
}

inline const SharedDbEnvPtr&
ConnectionI::dbEnv() const
{
    return _dbEnv;
}

inline const std::string& 
ConnectionI::envName() const
{
    return _envName;
}

inline const Ice::CommunicatorPtr&
ConnectionI::communicator() const
{
    return _communicator;
}

inline const Ice::EncodingVersion&
ConnectionI::encoding() const
{
    return _encoding;
}

inline Ice::Int
ConnectionI::trace() const
{
    return _trace;
}

inline Ice::Int
ConnectionI::txTrace() const
{
    return _txTrace;
}

inline bool
ConnectionI::deadlockWarning() const
{
    return _deadlockWarning;
}

}

#endif
