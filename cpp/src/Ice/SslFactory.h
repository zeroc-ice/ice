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

#include <IceUtil/Mutex.h>
#include <Ice/InstanceF.h>
#include <Ice/SystemInternalF.h>
#include <Ice/CertificateVerifierF.h>

#include <string>
#include <map>

namespace IceSSL
{

typedef std::map<void*, SystemInternalPtr> SslHandleSystemMap;

// This is defined as a class so as to ensure encapsulation.  We don't
// want just anybody creating System instances - when all this is moved
// out to a DLL/SO, we want to ensure that this vanilla interface is used
// to get whatever flavor of System the DLL/SO is designed to hand out.  As
// a result, different flavors of the Security Extension DLL/SO will have
// different definitions for getSystem().
class Factory
{
public:

    static SystemInternalPtr getSystem(const IceInternal::InstancePtr&);

    // System Handle related methods
    static void addSystemHandle(void*, const SystemInternalPtr&);
    static void removeSystemHandle(void*);
    static SystemInternalPtr getSystemFromHandle(void*);

private:

    static SslHandleSystemMap _sslHandleSystemRepository;
    static ::IceUtil::Mutex _systemRepositoryMutex;
};

}

#endif
