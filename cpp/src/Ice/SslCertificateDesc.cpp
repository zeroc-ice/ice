// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <openssl/ssl.h>
#include <Ice/SslCertificateDesc.h>

using namespace std;
using namespace IceSecurity::Ssl;

/////////////////////////
//// CertificateFile ////
/////////////////////////

IceSecurity::Ssl::CertificateFile::CertificateFile() :
                                  _fileName(""),
                                  _encoding(0)
{
}

IceSecurity::Ssl::CertificateFile::CertificateFile(const string& filename, const int encoding) :
                                  _fileName(filename),
                                  _encoding(encoding)
{
}

IceSecurity::Ssl::CertificateFile::CertificateFile(const CertificateFile& certFile) :
                                  _fileName(certFile._fileName),
                                  _encoding(certFile._encoding)
{
}

/////////////////////////////////
//// DiffieHellmanParamsFile ////
/////////////////////////////////

IceSecurity::Ssl::DiffieHellmanParamsFile::DiffieHellmanParamsFile() :
                                          CertificateFile(),
                                          _keySize(0)
{
}

IceSecurity::Ssl::DiffieHellmanParamsFile::DiffieHellmanParamsFile(const int keySize,
                                                                   const string& filename,
                                                                   const int encoding) :
                                          CertificateFile(filename, encoding),
                                          _keySize(keySize)
{
}

IceSecurity::Ssl::DiffieHellmanParamsFile::DiffieHellmanParamsFile(const DiffieHellmanParamsFile& dhParams) :
                                          CertificateFile(dhParams._fileName, dhParams._encoding),
                                          _keySize(dhParams._keySize)
{
}

/////////////////////////
//// CertificateDesc ////
/////////////////////////

IceSecurity::Ssl::CertificateDesc::CertificateDesc() :
                                  _keySize(0),
                                  _public(),
                                  _private()
{
}

IceSecurity::Ssl::CertificateDesc::CertificateDesc(const int keySize,
                                                   const CertificateFile& publicFile,
                                                   const CertificateFile& privateFile) :
                                  _keySize(keySize),
                                  _public(publicFile),
                                  _private(privateFile)
{
}

IceSecurity::Ssl::CertificateDesc::CertificateDesc(const CertificateDesc& certDesc) :
                                  _keySize(certDesc._keySize),
                                  _public(certDesc._public),
                                  _private(certDesc._private)
{
}

