// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

