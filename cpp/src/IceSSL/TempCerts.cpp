// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceSSL/TempCerts.h>

IceSSL::TempCertificates::TempCertificates()
{
}

IceSSL::TempCertificates::~TempCertificates()
{
    _rsaCerts.clear();
    _dhParams.clear();
}

void
IceSSL::TempCertificates::addRSACert(CertificateDesc& certDesc)
{
    _rsaCerts.push_back(certDesc);
}

void
IceSSL::TempCertificates::addDHParams(DiffieHellmanParamsFile& dhParams)
{
    _dhParams.push_back(dhParams);
}

IceSSL::RSAVector&
IceSSL::TempCertificates::getRSACerts()
{
    return _rsaCerts;
}

IceSSL::DHVector&
IceSSL::TempCertificates::getDHParams()
{
    return _dhParams;
}

