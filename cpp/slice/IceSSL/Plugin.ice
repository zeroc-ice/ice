// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_ICE
#define ICE_SSL_PLUGIN_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/Plugin.ice>
#include <IceSSL/CertificateVerifierF.ice>

/**
 *
 * &IceSSL; is a dynamic SSL transport plug-in for the &Ice; core. It
 * provides authentication, encryption, and message integrity, using
 * the industry-standard SSL protocol.
 *
 **/
module IceSSL
{

/**
 *
 * A [Plugin] may serve as a Client, Server or both (ClientServer).  A
 * <literal>Context</literal> is set up inside the [Plugin] in order to handle
 * either Client or Server roles.  The <literal>Context</literal> represents a
 * role-specific configuration.
 *
 * Some [Plugin] operations require a <literal>ContextType</literal> argument to
 * identify the <literal>Context</literal>.
 *
 **/
enum ContextType
{
    /** Select only the Client <literal>Context</literal>, no modifications to the Server. */
    Client,

    /** Select only the Server <literal>Context</literal>, no modifications to the Client. */
    Server,

    /** Select and affect changes on both the Client and Server <literal>Context</literal>s. */
    ClientServer
};

/**
 *
 * The interface for the SSL plug-in. This interface is typically
 * used to perform programmatic configuration of the plug-in.
 *
 **/
local interface Plugin extends Ice::Plugin
{
    /**
     *
     * Configure the plug-in. If the plug-in is left in an
     * unconfigured state, it will load its configuration from
     * the properties <literal>IceSSL.Server.Config</literal> or
     * <literal>IceSSL.Client.Config</literal>, depending on the context
     * type.
     *
     * Configuration property settings will also be loaded during
     * this operation, with the property values overriding those of the
     * configuration file.
     *
     * @param contextType The <literal>Context</literal>(s) to configure.
     *
     **/
    void configure(ContextType cType);

    /**
     *
     * Configure the plug-in for the given <literal>Context</literal>
     * using the settings in the given configuration file.
     *
     * If the plug-in is left in an unconfigured state, it
     * will load its configuration from the property
     * <literal>IceSSL.Server.Config</literal> or
     * <literal>IceSSL.Client.Config</literal>, depending on
     * the context type.
     *
     * Configuration property settings will also be loaded as part of
     * this operation, with the property values overriding those of the
     * configuration file.
     *
     * @param contextType The <literal>Context</literal> to configure.
     *
     * @param configFile The file containing the SSL configuration
     * information.
     *
     * @param certPath The path where certificates referenced in
     * [configFile] may be found.
     *
     **/
    void loadConfig(ContextType cType, string configFile, string certPath);

    /**
     *
     * Set the [CertificateVerifier] used for the indicated [ContextType]
     * role. All plug-in <literal>Context</literal>s are created with default
     * [CertificateVerifier] objects installed. Replacement
     * [CertificateVerifier]s can be specified using this operation.
     *
     * This operation only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The <literal>Context</literal>(s) in which to install the
     * Certificate Verifier.
     *
     * @param certVerifier The [CertificateVerifier] to install.
     *
     * @see IceSSL::CertificateVerifier
     *
     **/
    void setCertificateVerifier(ContextType cType, CertificateVerifier certVerifier);

    /**
     *
     * Add a trusted certificate to the plug-in's default certificate
     * store. The provided certificate (passed in Base64-encoded
     * binary DER format, as per the PEM format) is added to the
     * trust list so that the certificate, and all certificates signed
     * by its private key, are trusted.
     *
     * This method only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The <literal>Context</literal>(s) in which to add
     * the trusted certificate.
     *
     * @param certificate The certificate to be trusted, in Base64-encoded
     * binary DER format.
     *
     **/
    void addTrustedCertificateBase64(ContextType cType, string certificate);

    /**
     *
     * Add a trusted certificate to the plug-in's default certificate
     * store.  The provided certificate (passed in binary DER format)
     * is added to the trust list so that the certificate, and
     * all certificates signed by its private key, are trusted.
     *
     * This method only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The <literal>Context</literal>(s) in which to add
     * the trusted certificate.
     *
     * @param certificate The certificate, in binary DER format, to be trusted.
     *
     **/
    void addTrustedCertificate(ContextType cType, Ice::ByteSeq certificate);

    /**
     *
     * Set the RSA keys to be used by the plug-in when operating in
     * the context mode specified by [contextType].
     *
     * This method only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The <literal>Context</literal>(s) in which to
     * set/replace the RSA keys.
     *
     * @param privateKey The RSA private key, in Base64-encoded binary
     * DER format.
     *
     * @param publicKey The RSA public key, in Base64-encoded binary
     * DER format.
     *
     **/
    void setRSAKeysBase64(ContextType cType, string privateKey, string publicKey);

    /**
     *
     * Set the RSA keys to be used by the plug-in when operating in
     * the context mode specified by [contextType].
     *
     * This method only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The <literal>Context</literal>(s) in which to
     * set/replace the RSA keys.
     *
     * @param privateKey The RSA private key, in binary DER format.
     *
     * @param publicKey The RSA public key, in binary DER format.
     *
     **/
    void setRSAKeys(ContextType cType, Ice::ByteSeq privateKey, Ice::ByteSeq publicKey);

    /**
     *
     * Retrieves an instance of the [CertificateVerifier] that is
     * installed by default in all plug-in instances.
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
