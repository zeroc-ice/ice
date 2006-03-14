// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

