// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_TEMP_CERTS_H
#define ICE_SSL_TEMP_CERTS_H

#include <Ice/SslCertificateDesc.h>
#include <ostream>

namespace IceSecurity
{

namespace Ssl
{

class TempCertificates
{

public:
    TempCertificates();
    ~TempCertificates();

    void addRSACert(CertificateDesc&);
    void addDSACert(CertificateDesc&);
    void addDHParams(DiffieHellmanParamsFile&);

    inline RSAVector& getRSACerts() { return _rsaCerts; };
    inline DSAVector& getDSACerts() { return _dsaCerts; };
    inline DHVector& getDHParams() { return _dhParams; };

protected:
    RSAVector _rsaCerts;
    DSAVector _dsaCerts;
    DHVector _dhParams;
};

template<class Stream>
inline Stream& operator << (Stream& target, TempCertificates& tmpCerts)
{
    RSAVector::iterator iRSA = tmpCerts.getRSACerts().begin();
    RSAVector::iterator eRSA = tmpCerts.getRSACerts().end();

    while (iRSA != eRSA)
    {
        target << "RSA" << std::endl << "{" << std::endl;
        target << *iRSA;
        target << "}" << std::endl << std::endl;
        iRSA++;
    }

    DSAVector::iterator iDSA = tmpCerts.getDSACerts().begin();
    DSAVector::iterator eDSA = tmpCerts.getDSACerts().end();

    while (iDSA != eDSA)
    {
        target << "DSA" << std::endl << "{" << std::endl;
        target << *iDSA;
        target << "}" << std::endl << std::endl;
        iDSA++;
    }

    DHVector::iterator iDHP = tmpCerts.getDHParams().begin();
    DHVector::iterator eDHP = tmpCerts.getDHParams().end();

    while (iDHP != eDHP)
    {
        target << "DH" << std::endl << "{" << std::endl;
        target << *iDHP;
        target << "}" << std::endl << std::endl;
        iDHP++;
    }
    
    return target;
}

}

}

#endif
