// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CERTIFICATE_DESC_H
#define ICE_SSL_CERTIFICATE_DESC_H

#include <IceUtil/Config.h>
#include <openssl/ssl.h>
#include <vector>

namespace IceSSL
{

class CertificateFile
{
public:

    CertificateFile();
    CertificateFile(const std::string&, const int);
    CertificateFile(const CertificateFile&);

    std::string getFileName() const;
    int getEncoding() const;

protected:

    std::string _fileName;
    int _encoding;
};

class DiffieHellmanParamsFile : public CertificateFile
{
public:

    DiffieHellmanParamsFile();
    DiffieHellmanParamsFile(const int, const std::string&, const int);
    DiffieHellmanParamsFile(const DiffieHellmanParamsFile&);

    int getKeySize() const;

protected:

    int _keySize;
};

class CertificateDesc
{
public:

    CertificateDesc();
    CertificateDesc(const int, const CertificateFile&, const CertificateFile&);
    CertificateDesc(const CertificateDesc&);

    int getKeySize() const;

    const CertificateFile& getPublic() const;
    const CertificateFile& getPrivate() const;

protected:

    int _keySize;
    CertificateFile _public;
    CertificateFile _private;
};

typedef std::vector<CertificateDesc> RSAVector;
typedef std::vector<CertificateDesc> DSAVector;
typedef std::vector<DiffieHellmanParamsFile> DHVector;

template<class Stream>
inline Stream& operator << (Stream& target, const CertificateFile& certFile)
{
    if(certFile.getEncoding() == SSL_FILETYPE_PEM)
    {
        target << "[PEM]: " << certFile.getFileName();
    }
    else if(certFile.getEncoding() == SSL_FILETYPE_ASN1)
    {
        target << "[ASN1]: " << certFile.getFileName();
    }

    return target;
}

template<class Stream>
inline Stream& operator << (Stream& target, const DiffieHellmanParamsFile& dhParams)
{
    if(dhParams.getKeySize() != 0)
    {
        target << "Keysize: " << dhParams.getKeySize() << std::endl;
        target << "File:    " << ((CertificateFile&)dhParams) << std::endl;
    }

    return target;
}

template<class Stream>
inline Stream& operator << (Stream& target, const CertificateDesc& certDesc)
{
    if(certDesc.getKeySize() != 0)
    {
        target << "Keysize: " << certDesc.getKeySize() << std::endl;
        target << "Public:  " << certDesc.getPublic()  << std::endl;
        target << "Private: " << certDesc.getPrivate() << std::endl;
    }

    return target;
}

}

#endif
