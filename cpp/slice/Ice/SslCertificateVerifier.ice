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

module IceSecurity
{

module Ssl
{

/**
 * This is the base interface for objects that will perform Certificate
 * Verification during the SSL Handshake.  Subclass interfaces will
 * depend on the requirements of the underlying SSL implementation.
 *
 **/
local interface CertificateVerifier
{
};

};

};

#endif
