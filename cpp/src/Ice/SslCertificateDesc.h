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

#ifndef ICE_SSL_CERTIFICATE_DESC_H
#define ICE_SSL_CERTIFICATE_DESC_H

#include <string>
#include <vector>
#include <ostream>

namespace IceSecurity
{

namespace Ssl
{

class CertificateFile
{

public:
    CertificateFile();
    CertificateFile(const std::string&, const int);
    CertificateFile(const CertificateFile&);

    inline std::string getFileName() const { return _fileName; };
    inline int getEncoding() const { return _encoding; };

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

    inline int getKeySize() const { return _keySize; };

protected:
    int _keySize;
};

class CertificateDesc
{

public:
    CertificateDesc();
    CertificateDesc(const int, const CertificateFile&, const CertificateFile&);
    CertificateDesc(const CertificateDesc&);

    inline int getKeySize() const { return _keySize; };

    inline const CertificateFile& getPublic() const { return _public;  };
    inline const CertificateFile& getPrivate() const { return _private; };

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
    if (certFile.getEncoding() == SSL_FILETYPE_PEM)
    {
        target << "[PEM]: " << certFile.getFileName();
    }
    else if (certFile.getEncoding() == SSL_FILETYPE_ASN1)
    {
        target << "[ASN1]: " << certFile.getFileName();
    }

    return target;
}

template<class Stream>
inline Stream& operator << (Stream& target, const DiffieHellmanParamsFile& dhParams)
{
    if (dhParams.getKeySize() != 0)
    {
        target << "Keysize: " << dhParams.getKeySize() << std::endl;
        target << "File:    " << ((CertificateFile&)dhParams) << std::endl;
    }

    return target;
}

template<class Stream>
inline Stream& operator << (Stream& target, const CertificateDesc& certDesc)
{
    if (certDesc.getKeySize() != 0)
    {
        target << "Keysize: " << certDesc.getKeySize() << std::endl;
        target << "Public:  " << certDesc.getPublic()  << std::endl;
        target << "Private: " << certDesc.getPrivate() << std::endl;
    }

    return target;
}

}

}

#endif
