// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_TEMP_CERTS_H
#define ICE_SSL_TEMP_CERTS_H

#include <IceSSL/CertificateDesc.h>

namespace IceSSL
{

class TempCertificates
{
public:

    TempCertificates();
    ~TempCertificates();

    void addRSACert(CertificateDesc&);
    void addDHParams(DiffieHellmanParamsFile&);

    RSAVector& getRSACerts();
    DHVector& getDHParams();

protected:

    RSAVector _rsaCerts;
    DHVector _dhParams;
};

template<class Stream>
inline Stream& operator << (Stream& target, TempCertificates& tmpCerts)
{
    RSAVector::iterator iRSA = tmpCerts.getRSACerts().begin();
    RSAVector::iterator eRSA = tmpCerts.getRSACerts().end();

    while(iRSA != eRSA)
    {
        target << "RSA" << std::endl << "{" << std::endl;
        target << *iRSA;
        target << "}" << std::endl << std::endl;
        iRSA++;
    }

    DHVector::iterator iDHP = tmpCerts.getDHParams().begin();
    DHVector::iterator eDHP = tmpCerts.getDHParams().end();

    while(iDHP != eDHP)
    {
        target << "DH" << std::endl << "{" << std::endl;
        target << *iDHP;
        target << "}" << std::endl << std::endl;
        iDHP++;
    }
    
    return target;
}

}

#endif
