// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_EXTENSION_ICE
#define ICE_SSL_EXTENSION_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/SslCertificateVerifierF.ice>

module IceSSL
{

/**
 *
 * The [SslExtension] is the primary interface responsible for providing the
 * application layer with stock [CertificateVerifier] objects.
 *
 * @see CertificateVerifier
 *
 **/
local interface SslExtension
{
    /**
     *
     * Retrieves an instance of the stock [CertificateVerifier] that is
     * installed by default in all [System] instances.
     *
     * @return CertificateVerifier
     *
     **/
    CertificateVerifier getDefaultCertVerifier();

    /**
     *
     * Returns an instance of a [CertificateVerifier] that only accepts
     * a single certificate, that being the RSA certificate represented by
     * the binary DER encoding contained in the provided byte sequence.  This
     * is useful if you wish your application to accept connections from one
     * party.
     *
     * <note><para>Be sure to use the <literal>peer</literal>
     * <literal>verifymode</literal> in your SSL configuration file.</para>
     * </note>
     *
     * @param certificate A DER encoded RSA certificate.
     *
     * @return CertificateVerifier
     *
     **/
    CertificateVerifier getSingleCertVerifier(Ice::ByteSeq certificate);
};

};

#endif
