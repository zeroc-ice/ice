// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_OPENSSL_H
#define ICESSL_OPENSSL_H

#include <IceSSL/Plugin.h>

#include <openssl/x509v3.h>
#include <openssl/pem.h>

//
// Automatically link IceSSLOpenSSL[D|++11|++11D].lib with Visual C++
//
#if defined(_MSC_VER)
#  if !defined(ICE_BUILDING_ICESSL_OPENSSL) && defined(ICESSL_OPENSSL_API_EXPORTS)
#    define ICE_BUILDING_ICESSL_OPENSSL
#  endif

#  if !defined(ICE_BUILDING_ICESSL_OPENSSL)
#    pragma comment(lib, ICE_LIBNAME("IceSSLOpenSSL"))
#  endif
#endif

#ifndef ICESSL_OPENSSL_API
#   if defined(ICE_STATIC_LIBS)
#       define ICESSL_OPENSSL_API /**/
#   elif defined(ICESSL_OPENSSL_API_EXPORTS)
#       define ICESSL_OPENSSL_API ICE_DECLSPEC_EXPORT
#   else
#       define ICESSL_OPENSSL_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceSSL
{

namespace OpenSSL
{

class Certificate;
ICE_DEFINE_PTR(CertificatePtr, Certificate);

class ICESSL_OPENSSL_API Certificate : public virtual IceSSL::Certificate
{
public:

    //
    // Construct a certificate using a native certificate.
    //
    // The Certificate class assumes ownership of the given native
    // certificate.
    //
    static CertificatePtr create(x509_st*);

    //
    // Load the certificate from a file. The certificate must use the
    // PEM encoding format. Raises CertificateReadException if the
    // file cannot be read.
    //
    static CertificatePtr load(const std::string&);

    //
    // Decode a certificate from a string that uses the PEM encoding
    // format. Raises CertificateEncodingException if an error
    // occurs.
    //
    static CertificatePtr decode(const std::string&);

    //
    // Retrieve the native X509 certificate value wrapped by this
    // object.
    //
    // The returned reference is only valid for the lifetime of this
    // object. you can increment it with X509_dup.
    //
    virtual x509_st* getCert() const = 0;
};

class ICESSL_OPENSSL_API Plugin : public virtual IceSSL::Plugin
{
public:
    //
    // returns OpenSSL version number.
    //
    virtual Ice::Long getOpenSSLVersion() const = 0;
    //
    // Establish the OpenSSL context. This must be done before the
    // plug-in is initialized, therefore the application must define
    // the property Ice.InitPlugins=0, set the context, and finally
    // invoke initializePlugins on the PluginManager.
    //
    // When the application supplies its own OpenSSL context, the
    // plug-in ignores configuration properties related to certificates,
    // keys, and passwords.
    //
    // Note that the plugin assumes ownership of the given context.
    //
    virtual void setContext(SSL_CTX*) = 0;

    //
    // Obtain the SSL context. Use caution when modifying this value.
    // Changes made to this value have no effect on existing connections.
    //
    virtual SSL_CTX* getContext() = 0;
    
};
ICE_DEFINE_PTR(PluginPtr, Plugin);

} // OpenSSL namespace end

} // IceSSL namespace end

#endif
