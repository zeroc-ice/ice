// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/CertificateDesc.h>

using namespace std;
using namespace IceSSL;

/////////////////////////
//// CertificateFile ////
/////////////////////////

IceSSL::CertificateFile::CertificateFile() :
                        _fileName(""),
                        _encoding(SSL_FILETYPE_PEM)
{
}

IceSSL::CertificateFile::CertificateFile(const string& filename, const int encoding) :
                        _fileName(filename),
                        _encoding(encoding)
{
}

IceSSL::CertificateFile::CertificateFile(const CertificateFile& certFile) :
                        _fileName(certFile._fileName),
                        _encoding(certFile._encoding)
{
}

std::string
IceSSL::CertificateFile::getFileName() const
{
    return _fileName;
}

int
IceSSL::CertificateFile::getEncoding() const
{
    return _encoding;
}

/////////////////////////////////
//// DiffieHellmanParamsFile ////
/////////////////////////////////

IceSSL::DiffieHellmanParamsFile::DiffieHellmanParamsFile() :
                                CertificateFile(),
                                _keySize(0)
{
}

IceSSL::DiffieHellmanParamsFile::DiffieHellmanParamsFile(const int keySize,
                                                         const string& filename,
                                                         const int encoding) :
                                CertificateFile(filename, encoding),
                                _keySize(keySize)
{
}

IceSSL::DiffieHellmanParamsFile::DiffieHellmanParamsFile(const DiffieHellmanParamsFile& dhParams) :
                                CertificateFile(dhParams._fileName, dhParams._encoding),
                                _keySize(dhParams._keySize)
{
}

int
IceSSL::DiffieHellmanParamsFile::getKeySize() const
{
    return _keySize;
}

/////////////////////////
//// CertificateDesc ////
/////////////////////////

IceSSL::CertificateDesc::CertificateDesc() :
                        _keySize(0),
                        _public(),
                        _private()
{
}

IceSSL::CertificateDesc::CertificateDesc(const int keySize,
                                         const CertificateFile& publicFile,
                                         const CertificateFile& privateFile) :
                        _keySize(keySize),
                        _public(publicFile),
                        _private(privateFile)
{
}

IceSSL::CertificateDesc::CertificateDesc(const CertificateDesc& certDesc) :
                        _keySize(certDesc._keySize),
                        _public(certDesc._public),
                        _private(certDesc._private)
{
}

int
IceSSL::CertificateDesc::getKeySize() const
{
    return _keySize;
}

const CertificateFile&
IceSSL::CertificateDesc::getPublic() const
{
    return _public;
}

const CertificateFile&
IceSSL::CertificateDesc::getPrivate() const
{
    return _private;
}
