// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceSSL/SslConnection.h>

void ::IceInternal::incRef(::IceSSL::Connection* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::Connection* p) { p->__decRef(); }

IceSSL::Connection::Connection(const IceInternal::TraceLevelsPtr& traceLevels,
                               const Ice::LoggerPtr& logger,
                               const CertificateVerifierPtr& certificateVerifier) :
                   _traceLevels(traceLevels),
                   _logger(logger),
                   _certificateVerifier(certificateVerifier)
{
    assert(_traceLevels != 0);
    assert(_logger != 0);
    assert(_certificateVerifier != 0);
}

IceSSL::Connection::~Connection()
{
}

