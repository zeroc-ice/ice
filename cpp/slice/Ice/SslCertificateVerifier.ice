// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SECURITY_CERTIFICATE_VERIFIER_ICE
#define ICE_SECURITY_CERTIFICATE_VERIFIER_ICE

module IceSSL
{

/**
 *
 * This is the base interface for objects that will perform
 * Certificate Verification during the SSL handshake. Subclass
 * interfaces will depend on the requirements of the underlying SSL
 // ML: Derived interfaces.
 * implementation.
 // ML: I don't understand this explanation at all. What objects are you referring to?
 *
 **/
local interface CertificateVerifier
{
// ML: No methods??? What is this class? I guess this needs a lot more explanation.
};

};

#endif
