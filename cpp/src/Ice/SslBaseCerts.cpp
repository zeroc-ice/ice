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

using namespace IceSSL;

IceSSL::BaseCertificates::BaseCertificates()
{
}

IceSSL::BaseCertificates::BaseCertificates(CertificateDesc& rsaCert,
                                           CertificateDesc& dsaCert,
                                           DiffieHellmanParamsFile& dhParams) :
                         _rsaCert(rsaCert),
                         _dsaCert(dsaCert),
                         _dhParams(dhParams)
{
}

IceSSL::BaseCertificates::BaseCertificates(BaseCertificates& baseCerts) :
                         _rsaCert(baseCerts._rsaCert),
                         _dsaCert(baseCerts._dsaCert),
                         _dhParams(baseCerts._dhParams)
{
}

const IceSSL::CertificateDesc&
IceSSL::BaseCertificates::getRSACert() const
{
    return _rsaCert;
}

const IceSSL::CertificateDesc&
IceSSL::BaseCertificates::getDSACert() const
{
    return _dsaCert;
}

const IceSSL::DiffieHellmanParamsFile&
IceSSL::BaseCertificates::getDHParams() const
{
    return _dhParams;
}

