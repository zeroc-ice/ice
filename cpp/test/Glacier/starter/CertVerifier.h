// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GLACIER_CERT_VERIFIER_H
#define GLACIER_CERT_VERIFIER_H

#include <Ice/BuiltinSequences.h>
#include <Ice/SslCertificateVerifierOpenSSL.h>

using Ice::ByteSeq;

class CertVerifier : public IceSecurity::Ssl::OpenSSL::CertificateVerifier
{
public:
    CertVerifier(const ByteSeq&);

    virtual int verify(int, X509_STORE_CTX*, SSL*);

    ByteSeq toByteSeq(X509* certificate);

protected:
    ByteSeq _publicKey;
};

#endif

