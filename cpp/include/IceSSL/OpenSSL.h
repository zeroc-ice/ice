// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

#if defined(_WIN32) && !defined(ICESSL_OPENSSL_API_EXPORTS)

namespace Ice
{

/**
 * When using static libraries, calling this function ensures the OpenSSL version of the IceSSL plug-in is
 * linked with the application.
 * @param loadOnInitialize If true, the plug-in is loaded (created) during communicator initialization.
 * If false, the plug-in is only loaded during communicator initialization if its corresponding plug-in
 * property is set to 1.
 */
ICE_PLUGIN_REGISTER_DECLSPEC_IMPORT void registerIceSSLOpenSSL(bool loadOnInitialize = true);

}
#endif

namespace IceSSL
{

namespace OpenSSL
{

class Certificate;
ICE_DEFINE_PTR(CertificatePtr, Certificate);

/**
 * Encapsulates an OpenSSL X.509 certificate.
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_OPENSSL_API Certificate : public virtual IceSSL::Certificate
{
public:

    /**
     * Construct a certificate using a native certificate.
     * The Certificate class assumes ownership of the given native
     * certificate.
     * @param cert The native certificate.
     * @return A new certificate object.
     */
    static CertificatePtr create(x509_st* cert);

    /**
     * Load the certificate from a file. The certificate must use the
     * PEM encoding format.
     * @param file The certificate file.
     * @return A new certificate object.
     * @throws CertificateReadException if the file cannot be read.
     */
    static CertificatePtr load(const std::string& file);

    /**
     * Decode a certificate from a string that uses the PEM encoding format.
     * @param cert A string containing the PEM-encoded certificate.
     * @return A new certificate object.
     * @throws CertificateEncodingException if an error occurs.
     */
    static CertificatePtr decode(const std::string& cert);

    /**
     * Retrieve the native X509 certificate value wrapped by this object.
     * @return The native certificate. The returned reference is only valid for the lifetime of this
     * object. You can increment it with X509_dup.
     */
    virtual x509_st* getCert() const = 0;
};

/**
 * Represents the IceSSL plug-in object.
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_OPENSSL_API Plugin : public virtual IceSSL::Plugin
{
public:

    /**
     * Obtains the OpenSSL version number.
     * @return The version.
     */
    virtual Ice::Long getOpenSSLVersion() const = 0;

    /**
     * Establishes the OpenSSL context. This must be done before the
     * plug-in is initialized, therefore the application must define
     * the property Ice.InitPlugins=0, set the context, and finally
     * invoke Ice::PluginManager::initializePlugins.
     *
     * When the application supplies its own OpenSSL context, the
     * plug-in ignores configuration properties related to certificates,
     * keys, and passwords.
     *
     * Note that the plug-in assumes ownership of the given context.
     *
     * @param ctx The OpenSSL context.
     */
    virtual void setContext(SSL_CTX* ctx) = 0;

    /**
     * Obtains the SSL context. Use caution when modifying this value.
     * Changes made to this value have no effect on existing connections.
     * @return The OpenSSL context.
     */
    virtual SSL_CTX* getContext() = 0;
};
ICE_DEFINE_PTR(PluginPtr, Plugin);

} // OpenSSL namespace end

} // IceSSL namespace end

#endif
