// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_BASE_CERTS_H
#define ICE_SSL_BASE_CERTS_H

#include <IceSSL/CertificateDesc.h>

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
    if(baseCerts.getRSACert().getKeySize() != 0)
    {
        target << "RSA\n{\n";
	IceSSL::operator<<(target, baseCerts.getRSACert());
        target << "}\n\n";
    }

    if(baseCerts.getDSACert().getKeySize() != 0)
    {
        target << "DSA\n{\n";
	IceSSL::operator<<(target, baseCerts.getDSACert());
        target << "}\n\n";
    }

    if(baseCerts.getDHParams().getKeySize() != 0)
    {
        target << "DH\n{\n";
	IceSSL::operator<<(target, baseCerts.getDHParams());
        target << "}\n\n";
    }

    return target;
}

}

#endif
