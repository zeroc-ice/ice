// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslTempCerts.h>

IceSecurity::Ssl::TempCertificates::TempCertificates()
{
}

IceSecurity::Ssl::TempCertificates::~TempCertificates()
{
    _rsaCerts.clear();
    _dsaCerts.clear();
    _rsaCerts.clear();
}

void
IceSecurity::Ssl::TempCertificates::addRSACert(CertificateDesc& certDesc)
{
    _rsaCerts.push_back(certDesc);
}

void
IceSecurity::Ssl::TempCertificates::addDSACert(CertificateDesc& certDesc)
{
    _dsaCerts.push_back(certDesc);
}

void
IceSecurity::Ssl::TempCertificates::addDHParams(DiffieHellmanParamsFile& dhParams)
{
    _dhParams.push_back(dhParams);
}

