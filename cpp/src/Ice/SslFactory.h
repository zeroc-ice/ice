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
#include <Ice/SslSystem.h>

namespace IceSecurity
{

namespace Ssl
{

using std::string;
using std::map;

typedef map<string,System*> SystemMap;
typedef map<void*,System*> SslHandleSystemMap;

// This is defined as a class so as to ensure encapsulation.  We don't
// want just anybody creating System instances - when all this is moved
// out to a DLL/SO, we want to ensure that this vanilla interface is used
// to get whatever flavor of System the DLL/SO is designed to hand out.  As
// a result, different flavors of the Security Extension DLL/SO will have
// different definitions for getSystem().
class Factory
{

public:
    static System* getSystem(string&);
    static void releaseSystem(System*);

    static void addSystemHandle(void*, System*);
    static System* getSystemFromHandle(void*);
    static void releaseSystemFromHandle(void*, System*);

private:
    static SslHandleSystemMap _sslHandleSystemRepository;
    static SystemMap _systemRepository;
    static ::IceUtil::Mutex _systemRepositoryMutex;
};

}

}

#endif
