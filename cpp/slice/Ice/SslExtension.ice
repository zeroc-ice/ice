// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SECURITY_SSL_EXTENSION_ICE
#define ICE_SECURITY_SSL_EXTENSION_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/SslCertificateVerifierF.ice>

module IceSSL
{

/**
 * This is the primary interface responsible for providing the application
 * layer with stock certificate verification and configuration objects.
 *
 **/
local interface SslExtension
{
    /**
     * Retrieves an instance of the stock CertificateVerifier that is
     * installed by default in all System instances.
     **/
    CertificateVerifier getDefaultCertVerifier();

    /**
     * Returns an instance of a CertificateVerifier that only accepts
     * a single certificate, that being the certificate represented by
     * the binary DER encoding contained in the provided ByteSeq.
     **/
    CertificateVerifier getSingleCertVerifier(Ice::ByteSeq certificate);
};

};

#endif
