// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_FACTORY_H
#define ICE_SSL_FACTORY_H

#include <string>
#include <map>
#include <IceUtil/Mutex.h>
#include <Ice/SslSystemF.h>

namespace IceSecurity
{

namespace Ssl
{

using std::string;
using std::map;

typedef map<string, SystemPtr> SystemMap;
typedef map<void*, SystemPtr> SslHandleSystemMap;

// This is defined as a class so as to ensure encapsulation.  We don't
// want just anybody creating System instances - when all this is moved
// out to a DLL/SO, we want to ensure that this vanilla interface is used
// to get whatever flavor of System the DLL/SO is designed to hand out.  As
// a result, different flavors of the Security Extension DLL/SO will have
// different definitions for getSystem().
class Factory
{

public:
    static SystemPtr getSystem(string&);

    // System Handle related methods
    static void addSystemHandle(void*, const SystemPtr&);
    static void removeSystemHandle(void*);
    static SystemPtr getSystemFromHandle(void*);

private:
    static SslHandleSystemMap _sslHandleSystemRepository;
    static SystemMap _systemRepository;
    static ::IceUtil::Mutex _systemRepositoryMutex;
    static int _evict;

    static void reapSystems();
};

}

}

#endif
