// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_ICE
#define ICE_SSL_PLUGIN_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/Plugin.ice>
#include <IceSSL/CertificateVerifierF.ice>

module IceSSL
{

/**
 *
 *
 * A [Plugin] may serve as a Client, Server or both (ClientServer).  A
 * <literal>Context</literal> is set up inside the [Plugin] in order to handle
 * either Client or Server roles.  The <literal>Context</literal> represents a
 * role-specific configuration.
 *
 * When making calls against the [Plugin] that require us to identify which
 * <literal>Context</literal> to address, we use a <literal>ContextType</literal>
 * as a parameter in the call.
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
 * This is the main interface to a Communicator-specific instance of
 * the SSL subsystem, which supports programmatic configuration of the
 * SSL subsystem.
 *
 **/
local interface Plugin extends Ice::Plugin
{
    /**
     *
     * Tell the Plugin to configure itself. If the Plugin is left in a
     * non-configured state, it will load its configuration from
     * the properties <literal>IceSSL.Server.Config</literal> or
     * <literal>IceSSL.Client.Config</literal>, depending on the context
     * type.
     *
     * Configuration property settings will also be loaded as part of
     * this call, with the property values overriding those of the
     * configuration file.
     *
     * @param contextType The <literal>Context</literal>(s) to configure.
     *
     **/
    void configure(ContextType contextType);

    /**
     *
     * Tell the Plugin to configure the indicated <literal>Context</literal>
     * using the settings in the indicated configuration file.
     *
     * If the Plugin is left in a non-configured state, it
     * will load its configuration from the property
     * <literal>IceSSL.Server.Config</literal> or
     * <literal>IceSSL.Client.Config</literal>, depending on
     * the context type.
     *
     * Configuration property settings will also be loaded as part of
     * this call, with the property values overriding those of the
     * configuration file.
     *
     * @param contextType The <literal>Context</literal>(s) which to configure.
     *
     * @param configFile The file which contains the SSL configuration
     * information.
     *
     * @param certPath The path where certificates referenced in
     * [configFile] may be found.
     *
     **/
    void loadConfig(ContextType contextType, string configFile, string certPath);

    /**
     *
     * Set the [CertificateVerifier] used for the indicated [ContextType]
     * role. All Plugin <literal>Context</literal>s are created with default
     * [CertificateVerifier] objects configured. Replacement
     * [CertificateVerifier]s can be specified with this method.
     *
     * This method only affects new connections -- existing
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
    void setCertificateVerifier(ContextType contextType, CertificateVerifier certVerifier);

    /**
     *
     * Add a trusted certificate to the Plugin's default certificate
     * store. The provided certificate (passed in Base64 encoded
     * binary DER format, as per the PEM format) will be added to the
     * trust list, so that it, and all certificates signed by the
     * corresponding private key, will be trusted.
     *
     * This method only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The <literal>Context</literal>(s) in which to add
     * the trusted certificate.
     *
     * @param certificate The certificate, in Base64 encoded binary
     * DER format, to be trusted.
     *
     **/
    void addTrustedCertificateBase64(ContextType contextType, string certificate);

    /**
     *
     * Add a trusted certificate to the Plugin's default certificate
     * store.  The provided certificate (passed in binary DER format)
     * will be added to the trust list, so that it, and all certificates
     * signed by the corresponding private key, will be trusted.
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
    void addTrustedCertificate(ContextType contextType, Ice::ByteSeq certificate);

    /**
     *
     * Set the RSA keys to be used by the Plugin when operating in
     * the context mode specified by [contextType].
     *
     * This method only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The <literal>Context</literal>(s) in which to
     * set/replace the RSA keys.
     *
     * @param privateKey The RSA private key, in Base64 encoded binary
     * DER format.
     *
     * @param publicKey The RSA public key, in Base64 encoded binary
     * DER format.
     *
     **/
    void setRSAKeysBase64(ContextType contextType, string privateKey, string publicKey);

    /**
     *
     * Set the RSA keys to be used by the Plugin when operating in
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
    void setRSAKeys(ContextType contextType, Ice::ByteSeq privateKey, Ice::ByteSeq publicKey);

    /**
     *
     * Retrieves an instance of the stock [CertificateVerifier] that is
     * installed by default in all Plugin instances.
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
