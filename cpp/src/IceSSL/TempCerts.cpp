// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

