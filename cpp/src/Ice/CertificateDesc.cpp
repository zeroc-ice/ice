// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Note: This pragma is used to disable spurious warning messages having
//       to do with the length of debug symbols exceeding 255 characters.
//       This is due to STL template identifiers expansion.
//       The MSDN Library recommends that you put this pragma directive
//       in place to avoid the warnings.
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <Ice/CertificateDesc.h>

using namespace std;
using namespace IceSSL;

/////////////////////////
//// CertificateFile ////
/////////////////////////

IceSSL::CertificateFile::CertificateFile() :
                        _fileName(""),
                        _encoding(0)
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

