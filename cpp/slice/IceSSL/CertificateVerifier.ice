// **********************************************************************
//
// Copyright (c) 2002
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
