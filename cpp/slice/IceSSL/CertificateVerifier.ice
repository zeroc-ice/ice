// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_CERTIFICATE_VERIFIER_ICE
#define ICE_SSL_CERTIFICATE_VERIFIER_ICE

#include <IceSSL/Plugin.ice>

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
 * Default certificate verifier implementations can be obtained via the
 * [Plugin].
 *
 * As this is simply a base class for purposes of derivation, no methods are
 * defined.
 *
 * @see Plugin
 *
 **/
local interface CertificateVerifier
{
    /**
     *
     * Set the context type of this Certificate Verifier.
     *
     * @param type The type of context that is using this CertificateVerifier,
     * Client, Server or ClientServer.
     *
     **/
    void setContext(ContextType type);
};

};

#endif
