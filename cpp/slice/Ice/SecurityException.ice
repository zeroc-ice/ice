// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SECURITY_EXCEPTION_ICE
#define ICE_SECURITY_EXCEPTION_ICE

module IceSecurity
{

/**
 *
 * This exception represents the base of all security related exceptions
 * in Ice.  It is a local exception since usually a problem with security
 * precludes a proper secure connection to transmit exceptions upon.  As
 * well, many exceptions would contain information that is of no business
 * for external clients/servers.
 *
 **/
local exception SecurityException
{
    /**
     *
     * Contains pertinent information from the security system to help
     * explain the nature of the exception in greater detail.  In some
     * instances, it will contain information from the underlying security
     * implementation and/or debugging trace.
     *
     **/
    string _message;
};

module Ssl
{

/**
 *
 * This exception indicates that there has been a problem encountered
 * while parsing the SSL Configuration file, or while attempting to
 * locate the configuration file.  This exception could indicate a
 * problem with the <literal>Ice.Ssl.Config</literal> or
 * <literal>Ice.Ssl.CertPath</literal> properties for your
 * [Ice::Communicator].
 *
 **/
local exception ConfigParseException extends SecurityException
{
};

/**
 *
 * In a general sense, this exception indicates that there has been a
 * problem that has led to the shutdown of an SSL connection.
 *
 **/
local exception ShutdownException extends SecurityException
{
};

/**
 *
 * Indicates that a problem has been encountered that violates the SSL
 * protocol, causing the shutdown of the connection.
 *
 **/
local exception ProtocolException extends ShutdownException
{
};

/**
 *
 * This exception indicates that a problem was encoutnered validating
 * a client certificate during SSL protocol handshake.
 *
 **/
local exception CertificateException extends ShutdownException
{
};

module OpenSSL
{

/**
 *
 * A problem was encountered while setting up the
 * <literal>SSL_CTX</literal> context structure.  This can include
 * problems related to parsing the key files or allocating a
 * <literal>SSL_CTX</literal> structure.
 *
 **/
local exception ContextException extends SecurityException
{
};

};

};

};

#endif
