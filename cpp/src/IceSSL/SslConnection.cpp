// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceSSL/SslConnection.h>

void ::IceInternal::incRef(::IceSSL::Connection* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::Connection* p) { p->__decRef(); }

IceSSL::Connection::Connection(const TraceLevelsPtr& traceLevels,
                               const Ice::LoggerPtr& logger,
                               const CertificateVerifierPtr& certificateVerifier) :
    _traceLevels(traceLevels),
    _logger(logger),
    _certificateVerifier(certificateVerifier)
{
}

IceSSL::Connection::~Connection()
{
}

