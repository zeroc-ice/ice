// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SECURITY_SSL_SYSTEM_ICE
#define ICE_SECURITY_SSL_SYSTEM_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/SslCertificateVerifierF.ice>

module IceSSL
{

/**
 *
 * The Context Type for calls into the SSL System.  As an SSL System may serve
 * as a Client, Server or both, we should specify which roles the System supports
 * are modified by the call.
 *
 **/
enum ContextType
{
    /**
     *
     * Select only the Client context, no modifications to the Server.
     *
     **/
    Client,

    /**
     *
     * Select only the Server context, no modifications to the Client.
     *
     **/
    Server,

    /**
     *
     * Select and affect changes on both the Client and Server contexts.
     *
     **/
    ClientServer
};

/**
 * This is the main interface to a Communicator specific instance of the
 * SSL subsystem, which supports programatic configuration of the SSL
 * subsystem.
 *
 **/
local interface System
{
    /**
     *
     * Tell the System to configure itself. If the system is left in a
     * non-configured state, the system will load its configuration from
     * the properties IceSSL.Server.Config or IceSSL.Client.Config,
     * depending on the context type.
     *
     * Configuration property settings will also be loaded as part of this call,
     * with the property values overriding those of the configuration file.
     *
     * @param contextType The context(s) which to configure.
     *
     **/
    void configure(ContextType contextType);

    /**
     *
     * Tell the System to configure the indicated context using the settings in
     * the indicated configuration file.
     *
     * If the system is left in a non-configured state, the system will load
     * its configuration from the property Ice.Ssl.Server.Config or
     * Ice.Ssl.Client.Config, depending on the context type.
     *
     * Configuration property settings will also be loaded as part of this call,
     * with the property values overriding those of the configuration file.
     *
     * @param contextType The context(s) which to configure.
     *
     * @param configFile The file which contains the SSL configuration information.
     *
     * @param certPath The path where certificates referenced in configFile may be found.
     *
     **/
    void loadConfig(ContextType contextType, string configFile, string certPath);

    /**
     *
     * Set the CertificateVerifier used for the indicated ContextType role.
     * All System Contexts are created with default CertificateVerifier objects
     * configured.  Replacement CertificateVerifiers can be specified with this
     * method.
     *
     * This method only affects new connections - existing connections are
     * left unchanged.
     *
     * @param contextType The context(s) in which to install the Certificate Verifier.
     *
     * @param certVerifier The CertificateVerifier to install.
     *
     * @see IceSSL
     * @see IceSSL::CertificateVerifier
     *
     **/
    void setCertificateVerifier(ContextType contextType, CertificateVerifier certVerifier);

    /**
     *
     * Add a trusted certificate to the System's default certificate store.
     * The provided certificate (passsed in Base64 encoded binary DER format,
     * as per the PEM format) will be added to the trust list, so it, and all
     * certificates signed by the corresponding private key, will be trusted.
     *
     * This method only affects new connections - existing connections are
     * left unchanged.
     *
     * @param contextType The context(s) in which to add the trusted certificate.
     *
     * @param certificate The certificate, in Base64 encoded binary DER format, to be trusted.
     *
     * @see IceSSL
     *
     **/
    void addTrustedCertificate(ContextType contextType, string certificate);

    /**
     *
     * Set the RSA Keys to be used by the SSL System when operating in the
     * context mode specified by contextType.
     *
     * This method only affects new connections - existing connections are
     * left unchanged.
     *
     * @param contextType The context(s) in which to set/replace the RSA keys.
     *
     * @param privateKey The RSA private key, in Base64 encoded binary DER format.
     *
     * @param publicKey The RSA public key, in Base64 encoded binary DER format.
     *
     * @see IceSSL
     *
     **/
    void setRSAKeysBase64(ContextType contextType, string privateKey, string publicKey);

    /**
     *
     * Set the RSA Keys to be used by the SSL System when operating in the
     * context mode specified by contextType.
     *
     * This method only affects new connections - existing connections are
     * left unchanged.
     *
     * @param contextType The context(s) in which to set/replace the RSA keys.
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
