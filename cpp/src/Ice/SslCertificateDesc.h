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

using std::string;
using std::ostream;
using std::vector;

class CertificateFile
{

public:
    CertificateFile();
    CertificateFile(const string&, const int);
    CertificateFile(const CertificateFile&);

    inline string getFileName() const { return _fileName; };
    inline int getEncoding() const { return _encoding; };

protected:
    string _fileName;
    int _encoding;
};

class DiffieHellmanParamsFile : public CertificateFile
{

public:
    DiffieHellmanParamsFile();
    DiffieHellmanParamsFile(const int, const string&, const int);
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

typedef vector<CertificateDesc> RSAVector;
typedef vector<CertificateDesc> DSAVector;
typedef vector<DiffieHellmanParamsFile> DHVector;

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
        target << "Keysize: " << dhParams.getKeySize() << endl;
        target << "File:    " << ((CertificateFile&)dhParams) << endl;
    }

    return target;
}

template<class Stream>
inline Stream& operator << (Stream& target, const CertificateDesc& certDesc)
{
    if (certDesc.getKeySize() != 0)
    {
        target << "Keysize: " << certDesc.getKeySize() << endl;
        target << "Public:  " << certDesc.getPublic()  << endl;
        target << "Private: " << certDesc.getPrivate() << endl;
    }

    return target;
}

}

}

#endif
