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

namespace IceSecurity
{

namespace Ssl
{

class BaseCertificates
{

public:
    BaseCertificates();
    BaseCertificates(CertificateDesc&, CertificateDesc&, DiffieHellmanParamsFile&);
    BaseCertificates(BaseCertificates&);

    inline const CertificateDesc& getRSACert() const { return _rsaCert; };
    inline const CertificateDesc& getDSACert() const { return _dsaCert; };

    inline const DiffieHellmanParamsFile& getDHParams() const { return _dhParams; };

protected:
    CertificateDesc _rsaCert;
    CertificateDesc _dsaCert;
    DiffieHellmanParamsFile _dhParams;
};

using std::endl;

template<class Stream>
inline Stream& operator << (Stream& target, const BaseCertificates& baseCerts)
{
    if (baseCerts.getRSACert().getKeySize() != 0)
    {
        target << "RSA\n{" << endl;
        target << baseCerts.getRSACert();
        target << "}\n" << endl;
    }

    if (baseCerts.getDSACert().getKeySize() != 0)
    {
        target << "DSA\n{" << endl;
        target << baseCerts.getDSACert();
        target << "}\n" << endl;
    }

    if (baseCerts.getDHParams().getKeySize() != 0)
    {
        target << "DH\n{" << endl;
        target << baseCerts.getDHParams();
        target << "}\n" << endl;
    }

    return target;
}

}

}

#endif
