// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <Ice/SslFactory.h>
#include <Ice/SslSystemOpenSSL.h>

namespace IceSecurity
{

namespace Ssl
{

// Static member instantiations.
JTCMutex Factory::_systemRepositoryMutex;
SystemMap Factory::_systemRepository;
SslHandleSystemMap Factory::_sslHandleSystemRepository;

}

}

IceSecurity::Ssl::System*
IceSecurity::Ssl::Factory::getSystem(string& systemIdentifier)
{
    JTCSyncT<JTCMutex> sync(_systemRepositoryMutex);

    System* _system = _systemRepository[systemIdentifier];

    // Don't have that System.
    if (_system == 0)
    {
        // In our case, the systemIdentifier happens to be the
        // SSL Configuration file.

        // This line would change based on the flavor of System that we're
        // creating for the caller.
        _system = new OpenSSL::System(systemIdentifier);

        if (_system != 0)
        {
            _systemRepository[systemIdentifier] = _system;
        }
    }

    assert(_system);

    _system->incRef();

    return _system;
}

void
IceSecurity::Ssl::Factory::releaseSystem(System* system)
{
    JTCSyncT<JTCMutex> sync(_systemRepositoryMutex);

    assert(system);

    // If the reference count is now at zero.
    if (!system->decRef())
    {
        _systemRepository.erase(system->getSystemID());

        delete system;
    }
}

void
IceSecurity::Ssl::Factory::addSystemHandle(void* sslHandle, System* system)
{
    assert(sslHandle);
    assert(system);
    _sslHandleSystemRepository[sslHandle] = system;
}

IceSecurity::Ssl::System*
IceSecurity::Ssl::Factory::getSystemFromHandle(void* sslHandle)
{
    JTCSyncT<JTCMutex> sync(_systemRepositoryMutex);

    assert(sslHandle);

    System* _system = _sslHandleSystemRepository[sslHandle];

    assert(_system);

    _system->incRef();

    return _system;
}

void
IceSecurity::Ssl::Factory::releaseSystemFromHandle(void* sslHandle, System* system)
{
    JTCSyncT<JTCMutex> sync(_systemRepositoryMutex);

    assert(sslHandle);
    assert(system);

    // If the reference count is now at zero.
    if (!system->decRef())
    {
        _sslHandleSystemRepository.erase(sslHandle);
        _systemRepository.erase(system->getSystemID());

        delete system;
    }
}

