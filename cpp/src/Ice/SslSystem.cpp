// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <string>
#include <Ice/SslSystem.h>

using namespace std;
using IceSecurity::Ssl::CertificateVerifierPtr;
using Ice::LoggerPtr;
using Ice::PropertiesPtr;
using IceInternal::TraceLevelsPtr;

void ::IceInternal::incRef(::IceSecurity::Ssl::System* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSecurity::Ssl::System* p) { p->__decRef(); }

//
// Public Methods
//

void
IceSecurity::Ssl::System::setTrace(const TraceLevelsPtr& traceLevels)
{
    _traceLevels = traceLevels;
}

bool
IceSecurity::Ssl::System::isTraceSet() const
{
    return _traceLevels;
}

void
IceSecurity::Ssl::System::setLogger(const LoggerPtr& traceLevels)
{
    _logger = traceLevels;
}

bool
IceSecurity::Ssl::System::isLoggerSet() const
{
    return _logger;
}

void
IceSecurity::Ssl::System::setProperties(const PropertiesPtr& properties)
{
    _properties = properties;
}

bool
IceSecurity::Ssl::System::isPropertiesSet() const
{
    return _properties;
}

//
// Protected Methods
//

IceSecurity::Ssl::System::System()
{
}

IceSecurity::Ssl::System::~System()
{
}
