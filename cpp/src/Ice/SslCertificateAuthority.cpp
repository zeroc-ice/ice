// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslCertificateAuthority.h>

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
