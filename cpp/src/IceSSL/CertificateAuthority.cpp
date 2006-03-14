// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/CertificateAuthority.h>

using namespace std;

IceSSL::CertificateAuthority::CertificateAuthority()
{
}

IceSSL::CertificateAuthority::CertificateAuthority(string& fileName, string& path) :
    _fileName(fileName),
    _path(path)
{
}

IceSSL::CertificateAuthority::CertificateAuthority(CertificateAuthority& certAuthority) :
    _fileName(certAuthority._fileName),
    _path(certAuthority._path)
{
}

void
IceSSL::CertificateAuthority::setCAFileName(string& fileName)
{
    _fileName = fileName;
}

void
IceSSL::CertificateAuthority::setCAPath(string& caPath)
{
    _path = caPath;
}

const std::string&
IceSSL::CertificateAuthority::getCAFileName() const
{
    return _fileName;
}

const std::string&
IceSSL::CertificateAuthority::getCAPath() const
{
    return _path;
}
