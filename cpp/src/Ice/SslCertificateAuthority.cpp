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

IceSecurity::Ssl::CertificateAuthority::CertificateAuthority()
{
}

IceSecurity::Ssl::CertificateAuthority::CertificateAuthority(string& fileName, string& path) :
                                       _fileName(fileName),
                                       _path(path)
{
}

IceSecurity::Ssl::CertificateAuthority::CertificateAuthority(CertificateAuthority& certAuthority) :
                                       _fileName(certAuthority._fileName),
                                       _path(certAuthority._path)
{
}

void
IceSecurity::Ssl::CertificateAuthority::setCAFileName(string& fileName)
{
    _fileName = fileName;
}

void
IceSecurity::Ssl::CertificateAuthority::setCAPath(string& caPath)
{
    _path = caPath;
}
