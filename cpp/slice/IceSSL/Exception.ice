// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_EXCEPTION_ICE
#define ICE_SSL_EXCEPTION_ICE

module IceSSL
{

/**
 *
 * This exception represents the base of all security related exceptions
 * in &Ice;. It is a local exception because, usually, a problem with security
 * precludes a proper secure connection over which to transmit exceptions.
 * In addition, many exceptions would contain information that is of no use
 * to external clients/servers.
 *
 **/
local exception SslException
{
    /**
     *
     * Contains pertinent information from the security system to help
     * explain the nature of the exception in greater detail. In some
     * instances, it contains information from the underlying security
     * implementation and/or debugging trace.
     *
     **/
    string message;
};

/**
 *
 * This exception indicates that an attempt was made to load the
 * configuration for a <literal>Context</literal>, but the
 * property specifying the indicated <literal>Context</literal>'s 
 * SSL configuration file was not set. Check the values for the
 * appropriate property, either <literal>IceSSL.Client.Config</literal>
 * or <literal>IceSSL.Server.Config</literal>.
 *
 **/
local exception ConfigurationLoadingException extends SslException
{
};

/**
 *
 * This exception indicates that a problem occurred while parsing the
 * SSL configuration file, or while attempting to locate the configuration
 * file. This exception could indicate a
 * problem with the <literal>IceSSL.Client.Config</literal>,
 * <literal>IceSSL.Server.Config</literal>,
 * <literal>IceSSL.Client.CertPath</literal> or
 * <literal>IceSSL.Server.CertPath</literal> properties for your
 * [Ice::Communicator].
 *
 **/
local exception ConfigParseException extends SslException
{
};

/**
 *
 * This exception generally indicates that a problem occurred
 * that caused the shutdown of an SSL connection.
 *
 **/
local exception ShutdownException extends SslException
{
};

/**
 *
 * Indicates that a problem occurred that violates the SSL
 * protocol, causing the connection to be shutdown.
 *
 **/
local exception ProtocolException extends ShutdownException
{
};

/**
 *
 * Indicates a problem occurred during the certificate
 * verification phase of the SSL handshake. This is currently only
 * thrown by server connections.
 *
 **/
local exception CertificateVerificationException extends ShutdownException
{
};

/**
 *
 * A root exception class for all exceptions related to public key
 * certificates.
 *
 **/
local exception CertificateException extends SslException
{
};

/**
 *
 * Indicates that a problem occurred while signing certificates during
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
 * certificate into a form usable by the underlying SSL implementation.
 *
 **/
local exception CertificateParseException extends CertificateException
{
};

/**
 *
 * A root exception class for all exceptions related to private keys.
 *
 **/
local exception PrivateKeyException extends SslException
{
};

/**
 *
 * Indicates that IceSSL was unable to parse the provided private key
 * into a form usable by the underlying SSL implementation.
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

/**
 *
 * A problem was encountered while setting up the
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
 * Indicates that a problem occurred while initializing the context structure
 * of the underlying SSL implementation.
 *
 **/
local exception ContextInitializationException extends ContextException
{
};

/**
 *
 * This exception is raised when an attempt is made to make use of a
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
 *
 **/
local exception UnsupportedContextException extends ContextException
{
};

/**
 *
 * Indicates that a problem occurred while loading a certificate
 * into a <literal>Context</literal> from either a memory buffer
 * or from a file.
 *
 **/
local exception CertificateLoadException extends ContextException
{
};

/**
 *
 * Indicates that a problem occurred while loading a private key
 * into a <literal>Context</literal> from either a memory buffer
 * or from a file.
 *
 **/
local exception PrivateKeyLoadException extends ContextException
{
};

/**
 *
 * When loading a public and private key pair into a
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

#endif
