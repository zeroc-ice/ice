// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CERTIFICATE_VERIFIER_ICE
#define ICE_SSL_CERTIFICATE_VERIFIER_ICE

module IceSSL
{

/**
 *
 * The [CertificateVerifier] is the base interface for all classes that define
 * additional application-specific certificate verification rules.  These rules
 * are evaluated during the SSL handshake by an instance of a class derived
 * from [CertificateVerifier].  The methods defined in derived interfaces will
 * depend upon the requirements of the underlying SSL implementation.
 *
 * Existing stock certificate verifiers can be objtained by obtaining an
 * [SslExtension] instance and calling the appropriate method to obtain the
 * desired certificate verifier instance.
 *
 * As this is simply a base class for purposes of derivation, no methods are
 * defined.
 *
 * @see SslExtension
 *
 **/
local interface CertificateVerifier
{
};

};

#endif
