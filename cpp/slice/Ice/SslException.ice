// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_EXCEPTION_ICE
#define ICE_SSL_EXCEPTION_ICE

module IceSSL
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
local exception SslException
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

/**
 *
 * This exception indicates that an attempt was made to load the
 * configuration for a [System] <literal>Context</literal>, but the
 * property specifying the indicated <literal>Context</literal>'s 
 * SSL configuration file was not set.  Check the values for the
 * applicable property, either <literal>Ice.SSL.Client.Config</literal>
 * or <literal>Ice.SSL.Server.Config</literal>.
 *
 **/
local exception ConfigurationLoadingException extends SslException
{
};

/**
 *
 * This exception indicates that there has been a problem encountered
 * while parsing the SSL Configuration file, or while attempting to
 * locate the configuration file.  This exception could indicate a
 * problem with the <literal>Ice.SSL.Client.Config</literal>,
 * <literal>Ice.SSL.Server.Config</literal>,
 * <literal>Ice.SSL.Client.CertPath</literal> or
 * <literal>Ice.SSL.Server.CertPath</literal> properties for your
 * [Ice::Communicator].
 *
 **/
local exception ConfigParseException extends SslException
{
};

/**
 *
 * In a general sense, this exception indicates that there has been a
 * problem that has led to the shutdown of an SSL connection.
 *
 **/
local exception ShutdownException extends SslException
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
 * Thrown when a problem has been encountered during the certificate
 * verification phase of the SSL handshake.  This is currently only
 * thrown by server connections.
 *
 **/
local exception CertificateVerificationException extends ShutdownException
{
};

/**
 *
 * A root exception class for all exceptions that have to do explicity
 * with public key certificate related exceptions.
 *
 **/
local exception CertificateException extends SslException
{
};

/**
 *
 * Indicates that a problem was encountered signing certificates during
 * temporary RSA certificate generation.
 *
 **/
local exception CertificateSigningException extends CertificateException
{
};

/**
 *
 * Indicates that the signature verification of a newly signed temporary
 * RSA certificate has failed.
 *
 **/
local exception CertificateSignatureException extends CertificateException
{
};

/**
 *
 * Indicates that IceSSL was unable to parse the provided public key
 * certificate into the form used by the underlying SSL implementation.
 *
 **/
local exception CertificateParseException extends CertificateException
{
};

/**
 *
 * A root exception class for all exceptions that have to do explicity
 * with private key related exceptions.
 *
 **/
local exception PrivateKeyException extends SslException
{
};

/**
 *
 * Indicates that IceSSL was unable to parse the provided private key
 * into the form used by the underlying SSL implementation.
 *
 **/
local exception PrivateKeyParseException extends PrivateKeyException
{
};

/**
 *
 * This exception indicates that the provided CertificateVerifier was
 * not derived from the proper base class, and thus, does not provide
 * the appropriate interface.
 *
 */
local exception CertificateVerifierTypeException extends SslException
{
};

module OpenSSL
{

/**
 *
 * A problem was encountered while setting up the [IceSSL::System]
 * <literal>Context</literal>.  This can include problems related
 * to loading certificates and keys or calling methods on a
 * <literal>Context</literal> that has not been initialized as of yet.
 *
 **/
local exception ContextException extends SslException
{
};

/**
 *
 * This exception is generated when a problem was encountered initializing
 * the context structure of the underlying SSL implementation.
 *
 **/
local exception ContextInitializationException extends ContextException
{
};

/**
 *
 * This exception is thrown when an attempt is made to make a call on a
 * <literal>Context</literal> that has not been configured yet.
 *
 **/
local exception ContextNotConfiguredException extends ContextException
{
};

/**
 *
 * An attempt was made to call a method that references a
 * [IceSSL::ContextType] that is not supported for that operation.
 * This typically happens when an attempt is made to, for example,
 * request a [IceSSL::ClientServer] connection from a [IceSSL::System].
 *
 **/
local exception UnsupportedContextException extends ContextException
{
};

/**
 *
 * Generated when a problem was encountered loading a certificate
 * into a <literal>Context</literal> from either a memory buffer
 * or from a file.
 *
 **/
local exception CertificateLoadException extends ContextException
{
};

/**
 *
 * Generated when a problem was encountered loading a private key
 * into a <literal>Context</literal> from either a memory buffer
 * or from a file.
 *
 **/
local exception PrivateKeyLoadException extends ContextException
{
};

/**
 *
 * When loading a Public and Private key pair into a
 * <literal>Context</literal>, the load succeeded, but the private
 * key and public key (certificate) did not match.
 *
 **/
local exception CertificateKeyMatchException extends ContextException
{
};

/**
 *
 * An attempt to add a certificate to the <literal>Context</literal>'s
 * trusted certifificate store has failed.
 *
 **/
local exception TrustedCertificateAddException extends ContextException
{
};

};

};

#endif





