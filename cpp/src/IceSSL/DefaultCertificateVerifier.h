// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_DEFAULT_CERTIFICATE_VERIFIER_H
#define ICE_SSL_DEFAULT_CERTIFICATE_VERIFIER_H

#include <Ice/CommunicatorF.h>
#include <IceSSL/TraceLevelsF.h>
#include <IceSSL/CertificateVerifierOpenSSL.h>

namespace IceSSL
{

class DefaultCertificateVerifier : public IceSSL::CertificateVerifierOpenSSL
{
public:

    DefaultCertificateVerifier(const IceSSL::TraceLevelsPtr&, const Ice::CommunicatorPtr&);

    virtual int verify(int, X509_STORE_CTX*, SSL*);

private:

    IceSSL::TraceLevelsPtr _traceLevels;
    Ice::CommunicatorPtr _communicator;
};

}

#endif
