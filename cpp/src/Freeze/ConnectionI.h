// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

class ConnectionI : public Connection
{
public:

    virtual TransactionPtr beginTransaction();

    virtual TransactionPtr currentTransaction() const;

    virtual void close();
    
    virtual Ice::CommunicatorPtr getCommunicator() const;

    virtual std::string getName() const;

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

    const std::string&  envName() const;
    
    Ice::Int trace() const;

    Ice::Int txTrace() const;

    bool deadlockWarning() const;

private:

    Ice::CommunicatorPtr _communicator;
    SharedDbEnvPtr _dbEnv;
    std::string _envName;
    TransactionIPtr _transaction;
    std::list<MapHelperI*> _mapList;
    Ice::Int _trace;
    Ice::Int _txTrace;
    bool _deadlockWarning;
};  

inline void
ConnectionI::clearTransaction()
{
    _transaction = 0;
}

inline DbTxn*
ConnectionI::dbTxn() const
{
    if(_transaction == 0)
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
