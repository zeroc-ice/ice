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

    virtual TransactionPtr
    beginTransaction();

    virtual TransactionPtr
    currentTransaction() const;

    virtual void
    close();
    
    virtual Ice::CommunicatorPtr
    getCommunicator() const;

    virtual std::string
    getName() const;

    virtual ~ConnectionI();

    ConnectionI(const Ice::CommunicatorPtr& communicator, 
		const std::string& envName);

    ConnectionI(const Ice::CommunicatorPtr& communicator, 
		const std::string& envName,
		DbEnv& dbEnv);

    void
    closeAllIterators();

    void
    registerMap(MapHelperI*);

    void
    unregisterMap(MapHelperI*);

    void
    clearTransaction();

    DbTxn*
    dbTxn() const;

    DbEnv*
    dbEnv() const;
       
    const Ice::CommunicatorPtr&
    communicator() const;

    const std::string& 
    envName() const;
    
    Ice::Int
    trace() const;

    bool
    deadlockWarning() const;

private:

    Ice::CommunicatorPtr _communicator;
    SharedDbEnvPtr _dbEnvHolder;
    DbEnv* _dbEnv;
    std::string _envName;
    TransactionIPtr _transaction;
    std::list<MapHelperI*> _mapList;
    Ice::Int _trace;
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

inline DbEnv*
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

inline bool
ConnectionI::deadlockWarning() const
{
    return _deadlockWarning;
}

}

#endif
