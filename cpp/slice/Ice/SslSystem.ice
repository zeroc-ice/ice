// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// ML: Should be just ICE_SSL_SYSTEM_ICE
// ML: Ditto for all the other Ssl*.ice files.
#ifndef ICE_SECURITY_SSL_SYSTEM_ICE
#define ICE_SECURITY_SSL_SYSTEM_ICE

// ML: There should be more consistency in how @see is used (i.e., for
// what, and where).

#include <Ice/BuiltinSequences.ice>
#include <Ice/SslCertificateVerifierF.ice>

module IceSSL
{

/**
 *
 * The Context Type for calls into the SSL system. As an SSL system
 * may serve as a Client, Server or both, we should specify which
 * roles the System supports are modified by the call.
 // ML: I don't understand the sentence above. Please check grammar/meaning.
 *
 **/
enum ContextType
{
    /** Select only the Client context, no modifications to the Server. */
    Client,

    /** Select only the Server context, no modifications to the Client. */
    Server,

    /** Select and affect changes on both the Client and Server contexts. */
    ClientServer
};

/**
 *
 * This is the main interface to a Communicator specific instance of
 * the SSL subsystem, which supports programatic configuration of the
 * SSL subsystem.
 *
 **/
local interface System
{
    /**
     *
     * Tell the System to configure itself. If the system is left in a
     * non-configured state, the system will load its configuration
     * from the properties IceSSL.Server.Config or
     * IceSSL.Client.Config, depending on the context type.
     // ML: Use <literal>IceSSL.Client.Config</literal> and <literal>IceSSL.Server.Config</literal>.
     *
     * Configuration property settings will also be loaded as part of
     * this call, with the property values overriding those of the
     * configuration file.
     *
     * @param contextType The context(s) which to configure.
     *
     **/
    void configure(ContextType contextType);

    /**
     *
     * Tell the System to configure the indicated context using the
     * settings in the indicated configuration file.
     *
     * If the system is left in a non-configured state, the system
     * will load its configuration from the property
     * Ice.Ssl.Server.Config or Ice.Ssl.Client.Config, depending on
     // ML: Ditto.
     * the context type.
     *
     * Configuration property settings will also be loaded as part of
     * this call, with the property values overriding those of the
     * configuration file.
     *
     * @param contextType The context(s) which to configure.
     *
     * @param configFile The file which contains the SSL configuration
     * information.
     *
     * @param certPath The path where certificates referenced in
     * configFile may be found.
     // ML: Either [configFile] or configuration file.
     *
     **/
    void loadConfig(ContextType contextType, string configFile, string certPath);

    /**
     *
     * Set the CertificateVerifier used for the indicated ContextType
     // ML: Either Certificate Verifier, or [CertificateVerifier]. Same for ContextType.
     * role. All System Contexts are created with default
     * CertificateVerifier objects configured. Replacement
     // ML: Ditto.
     * CertificateVerifiers can be specified with this method.
     // ML: Certificate Verifiers or [CertificateVerifier]s.
     *
     * This method only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The context(s) in which to install the
     * Certificate Verifier.
     *
     * @param certVerifier The CertificateVerifier to install.
     // ML: Ditto.
     *
     * @see IceSSL
     * @see IceSSL::CertificateVerifier
     *
     **/
    void setCertificateVerifier(ContextType contextType, CertificateVerifier certVerifier);

    /**
     *
     * Add a trusted certificate to the System's default certificate
     * store.  The provided certificate (passed in Base64 encoded
     * binary DER format, as per the PEM format) will be added to the
     * trust list, so that it, and all certificates signed by the
     * corresponding private key, will be trusted.
     *
     * This method only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The context(s) in which to add the trusted
     * certificate.
     *
     * @param certificate The certificate, in Base64 encoded binary
     * DER format, to be trusted.
     *
     * @see IceSSL
     *
     **/
    void addTrustedCertificate(ContextType contextType, string certificate);

    /**
     *
     * Set the RSA keys to be used by the SSL system when operating in
     * the context mode specified by contextType.
     // ML: Either [contextType] or context type.
     *
     * This method only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The context(s) in which to set/replace the
     * RSA keys.
     *
     * @param privateKey The RSA private key, in Base64 encoded binary
     * DER format.
     *
     * @param publicKey The RSA public key, in Base64 encoded binary
     * DER format.
     *
     * @see IceSSL
     *
     **/
    void setRSAKeysBase64(ContextType contextType, string privateKey, string publicKey);

    /**
     *
     * Set the RSA keys to be used by the SSL system when operating in
     * the context mode specified by contextType.
     // ML: Either [contextType] or context type.
     *
     * This method only affects new connections -- existing
     * connections are left unchanged.
     *
     * @param contextType The context(s) in which to set/replace the
     * RSA keys.
     *
     * @param privateKey The RSA private key, in binary DER format.
     *
     * @param publicKey The RSA public key, in binary DER format.
     *
     * @see IceSSL
     *
     **/
    void setRSAKeys(ContextType contextType, Ice::ByteSeq privateKey, Ice::ByteSeq publicKey);
};

};

#endif
