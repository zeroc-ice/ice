// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SHARED_DB_ENV_H
#define FREEZE_SHARED_DB_ENV_H

#include <Freeze/Map.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Ice.h>
#include <db_cxx.h>
#include <map>
#include <list>

namespace Freeze
{

class CheckpointThread;
typedef IceUtil::Handle<CheckpointThread> CheckpointThreadPtr;

class SharedDbEnv;
typedef IceUtil::Handle<SharedDbEnv> SharedDbEnvPtr;

class MapDb;

class Transaction;
typedef IceInternal::Handle<Transaction> TransactionPtr;

class ConnectionI;
typedef IceUtil::Handle<ConnectionI> ConnectionIPtr;

class TransactionalEvictorContext;
typedef IceUtil::Handle<TransactionalEvictorContext> TransactionalEvictorContextPtr;

class SharedDbEnv
{
public:
    
    static SharedDbEnvPtr get(const Ice::CommunicatorPtr&, const std::string&, DbEnv* = 0);

    ~SharedDbEnv();

    //
    // Returns a shared map Db; the caller should NOT close/delete this Db.
    //
    MapDb* getSharedMapDb(const std::string&,const std::string&, const std::string&,
                          const KeyCompareBasePtr&, const std::vector<MapIndexBasePtr>&, bool);

    //
    // Tell SharedDbEnv to close and remove this Shared Db from the map
    //
    void removeSharedMapDb(const std::string&);

    
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
    const Ice::EncodingVersion& getEncoding() const;

    typedef std::map<std::string, MapDb*> SharedDbMap;

private:
    SharedDbEnv(const std::string&, const Ice::CommunicatorPtr&, DbEnv* env);
    
    void cleanup();

    DbEnv* _env;
    IceUtil::UniquePtr<DbEnv> _envHolder;
    const std::string _envName;
    const Ice::CommunicatorPtr _communicator;
    Ice::EncodingVersion _encoding;
    MapDb* _catalog;
    MapDb* _catalogIndexList;

    int _refCount;
    int _trace;
    CheckpointThreadPtr _thread;

#ifdef _WIN32
    DWORD _tsdKey;
#else
    pthread_key_t _tsdKey;
#endif

    SharedDbMap _sharedDbMap;
    IceUtil::Mutex _mutex;
    IceUtilInternal::FileLockPtr _fileLock;
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

inline const Ice::EncodingVersion& 
SharedDbEnv::getEncoding() const
{
    return _encoding;
}

}
#endif
