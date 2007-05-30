// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SHARED_DB_ENV_H
#define FREEZE_SHARED_DB_ENV_H

#include <Ice/Config.h>
#include <Ice/Ice.h>
#include <IceUtil/Handle.h>
#include <db_cxx.h>

namespace Freeze
{

class CheckpointThread;
typedef IceUtil::Handle<CheckpointThread> CheckpointThreadPtr;

class SharedDbEnv;
typedef IceUtil::Handle<SharedDbEnv> SharedDbEnvPtr;

class SharedDb;
typedef IceUtil::Handle<SharedDb> SharedDbPtr;

class Transaction;
typedef IceInternal::Handle<Transaction> TransactionPtr;

class TransactionalEvictorContext;
typedef IceUtil::Handle<TransactionalEvictorContext> TransactionalEvictorContextPtr;

class SharedDbEnv
{
public:
    
    static SharedDbEnvPtr get(const Ice::CommunicatorPtr&, const std::string&, DbEnv* = 0);

    ~SharedDbEnv();

    void __incRef();
    void __decRef();
   
    //
    // EvictorContext factory/manager
    //
    TransactionalEvictorContextPtr createCurrent();
    TransactionalEvictorContextPtr getCurrent();
    void setCurrentTransaction(const TransactionPtr& tx);

    DbEnv* getEnv() const;
    const std::string& getEnvName() const;
    const Ice::CommunicatorPtr& getCommunicator() const;
    const SharedDbPtr& getCatalog() const;

private:
    SharedDbEnv(const std::string&, const Ice::CommunicatorPtr&, DbEnv* env);
    
    DbEnv* _env;
    std::auto_ptr<DbEnv> _envHolder;
    const std::string _envName;
    const Ice::CommunicatorPtr _communicator;
    SharedDbPtr _catalog;

    int _refCount;
    int _trace;
    CheckpointThreadPtr _thread;

#ifdef _WIN32
    DWORD _tsdKey;
#else
    pthread_key_t _tsdKey;
#endif    

};

inline DbEnv*
SharedDbEnv::getEnv() const
{
    return _env;  
}

inline const std::string& 
SharedDbEnv::getEnvName() const
{
    return _envName;
}

inline const Ice::CommunicatorPtr& 
SharedDbEnv::getCommunicator() const
{
    return _communicator;
}

inline const SharedDbPtr&
SharedDbEnv::getCatalog() const
{
    return _catalog;
}


}
#endif
