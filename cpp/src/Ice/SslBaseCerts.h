// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_BASE_CERTS_H
#define ICE_SSL_BASE_CERTS_H

#include <Ice/SslCertificateDesc.h>
#include <ostream>

namespace IceSSL
{

class BaseCertificates
{
public:

    BaseCertificates();
    BaseCertificates(CertificateDesc&, CertificateDesc&, DiffieHellmanParamsFile&);
    BaseCertificates(BaseCertificates&);

    const CertificateDesc& getRSACert() const;
    const CertificateDesc& getDSACert() const;

    const DiffieHellmanParamsFile& getDHParams() const;

protected:

    CertificateDesc _rsaCert;
    CertificateDesc _dsaCert;
    DiffieHellmanParamsFile _dhParams;
};

template<class Stream>
inline Stream& operator << (Stream& target, const BaseCertificates& baseCerts)
{
    if (baseCerts.getRSACert().getKeySize() != 0)
    {
        target << "RSA\n{" << std::endl;
        target << baseCerts.getRSACert();
        target << "}\n" << std::endl;
    }

    if (baseCerts.getDSACert().getKeySize() != 0)
    {
        target << "DSA\n{" << std::endl;
        target << baseCerts.getDSACert();
        target << "}\n" << std::endl;
    }

    if (baseCerts.getDHParams().getKeySize() != 0)
    {
        target << "DH\n{" << std::endl;
        target << baseCerts.getDHParams();
        target << "}\n" << std::endl;
    }

    return target;
}

}

#endif
