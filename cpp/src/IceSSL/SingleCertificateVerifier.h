// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_SSL_SINGLE_CERTIFICATE_VERIFIER_H
#define ICE_SSL_SINGLE_CERTIFICATE_VERIFIER_H

#include <Ice/BuiltinSequences.h>
#include <IceSSL/CertificateVerifierOpenSSL.h>

namespace IceSSL
{

class SingleCertificateVerifier : public IceSSL::CertificateVerifierOpenSSL
{
public:

    SingleCertificateVerifier(const Ice::ByteSeq&);

    virtual int verify(int, X509_STORE_CTX*, SSL*);

    void toByteSeq(X509*, Ice::ByteSeq&);

protected:

    Ice::ByteSeq _publicKey;
};

}

#endif

