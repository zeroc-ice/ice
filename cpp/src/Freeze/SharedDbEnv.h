// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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


class SharedDbEnv : public ::DbEnv
{
public:
    
    static SharedDbEnvPtr get(const Ice::CommunicatorPtr&, const std::string&);

    ~SharedDbEnv();

    void __incRef();
    void __decRef();

    void deleteOldLogs();
   
    const std::string& getEnvName() const;
    const Ice::CommunicatorPtr& getCommunicator() const;

private:
    SharedDbEnv(const std::string&, const Ice::CommunicatorPtr&);
    
    const std::string _envName;
    const Ice::CommunicatorPtr _communicator;
    int _refCount;
    Ice::Int _trace;
    CheckpointThreadPtr _thread;
    IceUtil::Mutex _oldLogsMutex;
};

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

}
#endif
