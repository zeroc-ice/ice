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
