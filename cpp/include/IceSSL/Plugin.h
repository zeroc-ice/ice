// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_H
#define ICE_SSL_PLUGIN_H

#include <Ice/Plugin.h>

//
// SSL_CTX is the OpenSSL type that holds configuration settings for
// all SSL connections.
//
typedef struct ssl_ctx_st SSL_CTX;

//
// SSL is the OpenSSL type that represents an SSL connection.
//
typedef struct ssl_st SSL;

//
// X509 is the OpenSSL type that represents a certificate.
//
typedef struct x509_st X509;

namespace IceSSL
{

//
// VerifyInfo contains information that may be of use to a
// CertificateVerifier implementation.
//
struct VerifyInfo
{
    VerifyInfo();

    //
    // A value of true indicates an incoming (server) connection.
    //
    const bool incoming;

    //
    // The peer's certificate. This value may be 0 if the peer
    // did not supply a certificate.
    //
    X509* cert;

    //
    // The SSL connection object.
    //
    SSL* ssl;

    //
    // The address of the server as specified by the proxy's
    // endpoint. For example, in the following proxy:
    //
    // identity:ssl -h www.server.com -p 10000
    //
    // the value of address is "www.server.com".
    //
    // The value is an empty string for incoming connections.
    //
    const std::string address;

    //
    // The values of all dNSName and iPAddress fields in the peer
    // certificate's subjectAltName extension. An application may
    // use this information to restrict connections to peers that
    // have specific values.
    //
    const std::vector<std::string> dnsNames;
    const std::vector<std::string> ipAddresses;
};

//
// An application can customize the certificate verification process
// by implementing the CertificateVerifier interface.
//
class CertificateVerifier : public IceUtil::Shared
{
public:

    //
    // Return false if the connection should be rejected, or
    // true to allow it.
    //
    virtual bool verify(VerifyInfo&) = 0;
};
typedef IceUtil::Handle<CertificateVerifier> CertificateVerifierPtr;

//
// In order to read an encrypted file, such as one containing a private
// key, OpenSSL requests a password from IceSSL. The password can be
// defined using an IceSSL configuration property, but a plain-text
// password is a security risk. If a password is not supplied via
// configuration, IceSSL allows OpenSSL to prompt the user interactively.
// This may not be desirable (or even possible), so the application can
// supply an implementation of PasswordPrompt to take responsibility for
// obtaining the password.
//
// Note that the password is needed during plugin initialization, so in
// general you will need to delay initialization (by defining
// IceSSL.DelayInit=1), configure the PasswordPrompt, then manually
// initialize the plugin.
//
class PasswordPrompt : public IceUtil::Shared
{
public:

    //
    // The getPassword method may be invoked repeatedly, such as when
    // several encrypted files are opened, or when multiple password
    // attempts are allowed.
    //
    virtual std::string getPassword() = 0;
};
typedef IceUtil::Handle<PasswordPrompt> PasswordPromptPtr;

class Plugin : public Ice::Plugin
{
public:

    //
    // Initialize the IceSSL plugin. An application may supply its
    // own SSL_CTX objects to configure the SSL contexts for client
    // (outgoing) and server (incoming) connections. If an argument
    // is nonzero, the plugin skips its normal property-based
    // configuration.
    // 
    virtual void initialize(SSL_CTX* context = 0) = 0;

    //
    // Establish the certificate verifier object. This should be
    // done before any connections are established.
    //
    virtual void setCertificateVerifier(const CertificateVerifierPtr&) = 0;

    //
    // Establish the password prompt object. This must be done
    // before the plugin is initialized.
    //
    virtual void setPasswordPrompt(const PasswordPromptPtr&) = 0;

    //
    // Obtain the SSL context. If you need to customize the context,
    // you should do it before any SSL connections are established.
    //
    virtual SSL_CTX* context() = 0;
};
typedef IceUtil::Handle<Plugin> PluginPtr;

}

#endif
