// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslBaseCerts.h>

using namespace IceSecurity::Ssl;

IceSecurity::Ssl::BaseCertificates::BaseCertificates()
{
}

IceSecurity::Ssl::BaseCertificates::BaseCertificates(CertificateDesc& rsaCert,
                                                     CertificateDesc& dsaCert,
                                                     DiffieHellmanParamsFile& dhParams) :
                                   _rsaCert(rsaCert),
                                   _dsaCert(dsaCert),
                                   _dhParams(dhParams)
{
}

IceSecurity::Ssl::BaseCertificates::BaseCertificates(BaseCertificates& baseCerts) :
                                   _rsaCert(baseCerts._rsaCert),
                                   _dsaCert(baseCerts._dsaCert),
                                   _dhParams(baseCerts._dhParams)
{
}

