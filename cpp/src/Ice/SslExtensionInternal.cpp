// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslExtensionInternal.h>
#include <Ice/DefaultCertificateVerifier.h>
#include <Ice/SingleCertificateVerifier.h>
#include <Ice/BuiltinSequences.h>

IceSSL::SslExtensionInternal::SslExtensionInternal(const IceInternal::InstancePtr& instance) :
                             _instance(instance)
{
}

IceSSL::SslExtensionInternal::~SslExtensionInternal()
{
}

::IceSSL::CertificateVerifierPtr
IceSSL::SslExtensionInternal::getDefaultCertVerifier()
{
    return ::IceSSL::CertificateVerifierPtr(new IceSSL::OpenSSL::DefaultCertificateVerifier(_instance));
}

::IceSSL::CertificateVerifierPtr
IceSSL::SslExtensionInternal::getSingleCertVerifier(const ::Ice::ByteSeq& certSeq)
{
    return ::IceSSL::CertificateVerifierPtr(new IceSSL::OpenSSL::SingleCertificateVerifier(certSeq));
}

