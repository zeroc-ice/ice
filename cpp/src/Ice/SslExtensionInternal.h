// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_EXTENSION_INTERNAL_H
#define ICE_SSL_EXTENSION_INTERNAL_H

#include <Ice/InstanceF.h>
#include <Ice/SslExtension.h>
#include <Ice/CertificateVerifierF.h>

namespace IceSSL
{

class SslExtensionInternal : public SslExtension
{
public:

    SslExtensionInternal(const IceInternal::InstancePtr&);
    virtual ~SslExtensionInternal();

    virtual ::IceSSL::CertificateVerifierPtr getDefaultCertVerifier();

    virtual ::IceSSL::CertificateVerifierPtr getSingleCertVerifier(const ::Ice::ByteSeq&);

protected:

    IceInternal::InstancePtr _instance;
};

}

#endif
