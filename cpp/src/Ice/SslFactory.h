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
#include <Ice/SslCertificateVerifierF.h>

#ifdef WIN32
#   ifdef ICE_API_EXPORTS
#       define ICE_API __declspec(dllexport)
#   else
#       define ICE_API __declspec(dllimport)
#   endif
#else
#   define ICE_API /**/
#endif

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
    static SystemPtr getSystem(const string&);

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

// TODO: This is NOT how this should be done, but to get us over the hump for the
//       time being, we'll take this shortcut.

typedef enum
{
    None = 0,
    Client,
    Server,
    ClientServer
} ICE_API CertificateVerifierType;


void ICE_API setSystemCertificateVerifier(const string&, CertificateVerifierType, const CertificateVerifierPtr&);

}

}

#endif
