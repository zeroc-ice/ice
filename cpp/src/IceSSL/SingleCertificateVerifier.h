// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    Ice::ByteSeq toByteSeq(X509*);

protected:

    Ice::ByteSeq _publicKey;
};

}

#endif

