// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Note: This pragma is used to disable spurious warning messages having
//       to do with the length of debug symbols exceeding 255 characters.
//       This is due to STL template identifiers expansion.
//       The MSDN Library recommends that you put this pragma directive
//       in place to avoid the warnings.
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <Ice/Instance.h>
#include <Ice/SslFactory.h>
#include <Ice/SslSystemOpenSSL.h>
#include <Ice/OpenSSL.h>

#define OPENSSL_THREAD_DEFINES
#include <openssl/opensslconf.h>
#if defined(THREADS)
#else
#error "Thread support not enabled"
#endif


namespace IceSSL
{

// Static member instantiations.
IceUtil::Mutex Factory::_systemRepositoryMutex;
SslHandleSystemMap Factory::_sslHandleSystemRepository;

extern "C"
{
    void lockingCallback(int, int, const char*, int);
}

class SslLockKeeper
{

public:
    SslLockKeeper()
    {
        CRYPTO_set_locking_callback((void (*)(int, int, const char*, int))lockingCallback);
    }

    ~SslLockKeeper()
    {
        CRYPTO_set_locking_callback(NULL);
    }

    IceUtil::Mutex sslLocks[CRYPTO_NUM_LOCKS];

};

SslLockKeeper lockKeeper;

}

void IceSSL::lockingCallback(int mode, int type, const char *file, int line)
{
    if (mode & CRYPTO_LOCK)
    {
        lockKeeper.sslLocks[type].lock();
    }
    else
    {
        lockKeeper.sslLocks[type].unlock();
    }
}

IceSSL::SystemInternalPtr
IceSSL::Factory::getSystem(const IceInternal::InstancePtr& instance)
{
    SystemInternalPtr system = new OpenSSL::System(instance);

    assert(system != 0);

    return system;
}

void
IceSSL::Factory::addSystemHandle(void* sslHandle, const SystemInternalPtr& system)
{
    IceUtil::Mutex::Lock sync(_systemRepositoryMutex);

    assert(system != 0);
    assert(sslHandle != 0);
    _sslHandleSystemRepository[sslHandle] = system;
}

void
IceSSL::Factory::removeSystemHandle(void* sslHandle)
{
    IceUtil::Mutex::Lock sync(_systemRepositoryMutex);

    assert(sslHandle != 0);
    _sslHandleSystemRepository.erase(sslHandle);
}

IceSSL::SystemInternalPtr
IceSSL::Factory::getSystemFromHandle(void* sslHandle)
{
    IceUtil::Mutex::Lock sync(_systemRepositoryMutex);

    assert(sslHandle != 0);

    SystemInternalPtr& system = _sslHandleSystemRepository[sslHandle];

    assert(system != 0);

    return system;
}

