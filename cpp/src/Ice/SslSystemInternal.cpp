// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Instance.h>
#include <Ice/SslSystemInternal.h>
#include <string>

using namespace std;
using IceSSL::CertificateVerifierPtr;
using Ice::LoggerPtr;
using Ice::PropertiesPtr;
using IceInternal::TraceLevelsPtr;

void ::IceInternal::incRef(::IceSSL::SystemInternal* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::SystemInternal* p) { p->__decRef(); }

//
// Protected Methods
//

IceSSL::SystemInternal::SystemInternal(const IceInternal::InstancePtr& instance) :
                       _traceLevels(instance->traceLevels()),
                       _logger(instance->logger()),
                       _properties(instance->properties())
{
    assert(_traceLevels != 0);
    assert(_logger != 0);
    assert(_properties != 0);
}

IceSSL::SystemInternal::~SystemInternal()
{
}
