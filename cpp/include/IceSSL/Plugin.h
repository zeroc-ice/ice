// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_H
#define ICE_SSL_PLUGIN_H

#include <IceUtil/Time.h>
#include <Ice/Plugin.h>
#include <IceSSL/Config.h>
#include <IceSSL/ConnectionInfo.h>

#include <vector>
#include <list>

// For struct sockaddr_storage
#ifdef _WIN32
#   include <winsock2.h>
#else
#   include <sys/socket.h>
#endif

#if defined(ICE_USE_SECURE_TRANSPORT)
#   include <CoreFoundation/CFError.h>
#elif defined(ICE_USE_SCHANNEL)
#   include <wincrypt.h>
#endif

#ifndef ICE_SSL_API
#   ifdef ICE_SSL_API_EXPORTS
#       define ICE_SSL_API ICE_DECLSPEC_EXPORT
#   elif defined(ICE_STATIC_LIBS)
#       define ICE_SSL_API /**/
#   else
#       define ICE_SSL_API ICE_DECLSPEC_IMPORT
#   endif
#endif

#ifdef ICE_USE_OPENSSL

//
// Pointer to an opaque SSL session context object. ssl_ctx_st is the
// OpenSSL type that holds configuration settings for all SSL
// connections.
//
typedef struct ssl_ctx_st SSL_CTX;

//
// Pointer to an opaque certificate object. X509_st is the OpenSSL
// type that represents a certificate.
//
typedef struct x509_st* X509CertificateRef;

//
// EVP_PKEY is the OpenSSL type that represents a public key.
//
typedef struct evp_pkey_st* KeyRef;

//
// Type that represents an X509 distinguished name
//
typedef struct X509_name_st X509NAME;

#elif defined(ICE_USE_SECURE_TRANSPORT)

//
// Pointer to an opaque certificate object.
//
struct OpaqueSecCertificateRef;
typedef struct OpaqueSecCertificateRef* X509CertificateRef;

//
// Pointer to an opaque key object.
//
struct OpaqueSecKeyRef;
typedef struct OpaqueSecKeyRef* KeyRef;

#elif defined(ICE_USE_SCHANNEL)

//
// Pointer to an opaque certificate object.
//
typedef CERT_SIGNED_CONTENT_INFO* X509CertificateRef;

//
// Pointer to an opaque key object.
//
typedef CERT_PUBLIC_KEY_INFO* KeyRef;

#endif

namespace IceSSL
{

//
// This exception is thrown if the certificate cannot be read.
//
class ICE_SSL_API CertificateReadException : public IceUtil::Exception
{
public:

    CertificateReadException(const char*, int, const std::string&);
    virtual ~CertificateReadException() throw();
    virtual std::string ice_name() const;
    virtual CertificateReadException* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason;

private:

    static const char* _name;
};

//
// This exception is thrown if the certificate cannot be encoded.
//
class ICE_SSL_API CertificateEncodingException : public IceUtil::Exception
{
public:

    CertificateEncodingException(const char*, int, const std::string&);
#ifdef ICE_USE_SECURE_TRANSPORT
    CertificateEncodingException(const char*, int, CFErrorRef);
#endif
    virtual ~CertificateEncodingException() throw();
    virtual std::string ice_name() const;
    virtual CertificateEncodingException* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason;

private:

    static const char* _name;
};

//
// This exception is thrown if a distinguished name cannot be parsed.
//
class ICE_SSL_API ParseException : public IceUtil::Exception
{
public:

    ParseException(const char*, int, const std::string&);
    virtual ~ParseException() throw();
    virtual std::string ice_name() const;
    virtual ParseException* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason;

private:

    static const char* _name;
};

//
// Forward declaration.
//
class Certificate;
typedef IceUtil::Handle<Certificate> CertificatePtr;

//
// A representation of a PublicKey.
//
class ICE_SSL_API PublicKey : public IceUtil::Shared
{
public:

    ~PublicKey();

    //
    // Retrieve the native public key value wrapped by this object.
    //
    // The returned reference is only valid for the lifetime of this
    // object. With SecureTransport you can increment the reference
    // count of the returned object with CFRetain.
    //
    KeyRef key() const;

private:

    PublicKey(const CertificatePtr&, KeyRef);
    friend class Certificate;

    CertificatePtr _cert;
    KeyRef _key;

};
typedef IceUtil::Handle<PublicKey> PublicKeyPtr;

//
// This class represents a DistinguishedName, similar to the Java
// type X500Principal and the .NET type X500DistinguishedName.
//
// For comparison purposes, the value of a relative distinguished
// name (RDN) component is always unescaped before matching,
// therefore "ZeroC, Inc." will match ZeroC\, Inc.
//
// toString() always returns exactly the same information as was
// provided in the constructor (i.e., "ZeroC, Inc." will not turn
// into ZeroC\, Inc.).
//
class ICE_SSL_API DistinguishedName
{
public:

#ifdef ICE_USE_OPENSSL
    //
    // Create a DistinguishedName using an OpenSSL value.
    //
    DistinguishedName(X509NAME*);
#endif

    //
    // Create a DistinguishedName from a string encoded using
    // the rules in RFC2253.
    //
    // Throws ParseException if parsing fails.
    //
    DistinguishedName(const std::string&);

    //
    // Create a DistinguishedName from a list of RDN pairs,
    // where each pair consists of the RDN's type and value.
    // For example, the RDN "O=ZeroC" is represented by the
    // pair ("O", "ZeroC").
    //
    DistinguishedName(const std::list<std::pair<std::string, std::string> >&);

    //
    // This is an exact match. The order of the RDN components is
    // important.
    //
    bool operator==(const DistinguishedName&) const;
    bool operator!=(const DistinguishedName&) const;
    bool operator<(const DistinguishedName&) const;

    //
    // Perform a partial match with another DistinguishedName. The function
    // returns true if all of the RDNs in the argument are present in this
    // DistinguishedName and they have the same values.
    //
    bool match(const DistinguishedName&) const;

    //
    // Encode the DN in RFC2253 format.
    //
    operator std::string() const;

private:

    void unescape();

    std::list<std::pair<std::string, std::string> > _rdns;
    std::list<std::pair<std::string, std::string> > _unescaped;
};

//
// This convenience class is a wrapper around a native certificate.
// The interface is inspired by java.security.cert.X509Certificate.
//
class ICE_SSL_API Certificate : public IceUtil::Shared
{
public:

    //
    // Construct a certificate using a native certificate.
    //
    // The Certificate class assumes ownership of the given native
    // certificate.
    //
    Certificate(X509CertificateRef);
    ~Certificate();

    //
    // Load the certificate from a file. The certificate must use the
    // PEM encoding format. Raises CertificateReadException if the
    // file cannot be read.
    //
    static CertificatePtr load(const std::string&);

    //
    // Decode a certificate from a string that uses the PEM encoding
    // format.  Raises CertificateEncodingException if an error
    // occurs.
    //
    static CertificatePtr decode(const std::string&);

    //
    // Those operators compare the certificates for equality using the
    // native certificate comparison method.
    //
    bool operator==(const Certificate&) const;
    bool operator!=(const Certificate&) const;

    //
    // Get the certificate's public key.
    //
    PublicKeyPtr getPublicKey() const;

    //
    // Verify that this certificate was signed by the given certificate
    // public key. Returns true if signed, false otherwise.
    //
    bool verify(const CertificatePtr&) const;

#ifdef ICE_USE_OPENSSL
    //
    // Verify that this certificate was signed by the given public
    // key. Returns true if signed, false otherwise.
    //
    // This method was deprecated for consistency with some SSL
    // engines that require a certificate and not just a public key to
    // verify the certificate signature.
    //
    ICE_DEPRECATED_API("verify(const PublicKeyPtr&) is deprecated, use verify(const CertificatePtr&) instead")
    bool verify(const PublicKeyPtr&) const;
#endif

    //
    // Return a string encoding of the certificate in PEM format.
    // Raises CertificateEncodingException if an error occurs.
    //
    std::string encode() const;

    //
    // Checks that the certificate is currently valid, that is, the current
    // date falls between the validity period given in the certificate.
    //
    bool checkValidity() const;

    //
    // Checks that the certificate is valid at the given time.
    //
    bool checkValidity(const IceUtil::Time&) const;

    //
    // Get the not-after validity time.
    //
    IceUtil::Time getNotAfter() const;

    //
    // Get the not-before validity time.
    //
    IceUtil::Time getNotBefore() const;

    //
    // Get the serial number. This is an arbitrarily large number.
    //
    std::string getSerialNumber() const;

    //
    // Get the signature algorithm name used to sign the certificate.
    //
    //std::string getSigAlgName() const;

    //
    // Get the signature algorithm OID string from the certificate.
    //
    //std::string getSigAlgOID() const;

    //
    // Get the issuer's distinguished name (DN).
    //
    DistinguishedName getIssuerDN() const;

    //
    // Get the values in the issuer's alternative names extension.
    //
    // The returned list contains a pair of int, string.
    //
    // otherName                       [0]     OtherName
    // rfc822Name                      [1]     IA5String
    // dNSName                         [2]     IA5String
    // x400Address                     [3]     ORAddress
    // directoryName                   [4]     Name
    // ediPartyName                    [5]     EDIPartyName
    // uniformResourceIdentifier       [6]     IA5String
    // iPAddress                       [7]     OCTET STRING
    // registeredID                    [8]     OBJECT IDENTIFIER
    //
    // rfc822Name, dNSName, directoryName and
    // uniformResourceIdentifier data is returned as a string.
    //
    // iPAddress is returned in dotted quad notation. IPv6 is not
    // currently supported.
    //
    // All distinguished names are encoded in RFC2253 format.
    //
    // The remainder of the data will result in an empty string. Use the raw
    // X509* certificate to obtain these values.
    //
    std::vector<std::pair<int, std::string> > getIssuerAlternativeNames();

    //
    // Get the subject's distinguished name (DN).
    //
    DistinguishedName getSubjectDN() const;

    //
    // See the comment for getIssuerAlternativeNames.
    //
    std::vector<std::pair<int, std::string> > getSubjectAlternativeNames();

    //
    // Retrieve the certificate version number.
    //
    int getVersion() const;

    //
    // Stringify the certificate. This is a human readable version of
    // the certificate, not a DER or PEM encoding.
    //
    std::string toString() const;

    //
    // Retrieve the native X509 certificate value wrapped by this
    // object.
    //
    // The returned reference is only valid for the lifetime of this
    // object. With SecureTransport you can increment the reference
    // count of the returned object with CFRetain. With OpenSSL, you
    // can increment it with X509_dup. With SChannel, the returned
    // reference is a pointer to a struct.
    //
    X509CertificateRef getCert() const;

private:

    X509CertificateRef _cert;

#ifdef ICE_USE_SCHANNEL
    CERT_INFO* _certInfo;
#endif
};

//
// NativeConnectionInfo is an extension of IceSSL::ConnectionInfo that
// provides access to native certificates.
//
class ICE_SSL_API NativeConnectionInfo : public ConnectionInfo
{
public:

    //
    // The certificate chain. This may be empty if the peer did not
    // supply a certificate. The peer's certificate (if any) is the
    // first one in the chain.
    //
    std::vector<CertificatePtr> nativeCerts;
};
typedef IceUtil::Handle<NativeConnectionInfo> NativeConnectionInfoPtr;

//
// WSSNativeConnectionInfo is an extension of IceSSL::WSSConnectionInfo
// that provides access to native certificates.
//
class ICE_SSL_API WSSNativeConnectionInfo : public WSSConnectionInfo
{
public:

    //
    // The certificate chain. This may be empty if the peer did not
    // supply a certificate. The peer's certificate (if any) is the
    // first one in the chain.
    //
    std::vector<CertificatePtr> nativeCerts;
};
typedef IceUtil::Handle<WSSNativeConnectionInfo> WSSNativeConnectionInfoPtr;

//
// An application can customize the certificate verification process
// by implementing the CertificateVerifier interface.
//
class ICE_SSL_API CertificateVerifier : public IceUtil::Shared
{
public:

    //
    // Return false if the connection should be rejected, or true to
    // allow it.
    //
    virtual bool verify(const NativeConnectionInfoPtr&) = 0;
};
typedef IceUtil::Handle<CertificateVerifier> CertificateVerifierPtr;

//
// In order to read an encrypted file, such as one containing a
// private key, OpenSSL requests a password from IceSSL. The password
// can be defined using an IceSSL configuration property, but a
// plain-text password is a security risk. If a password is not
// supplied via configuration, IceSSL allows OpenSSL to prompt the
// user interactively.  This may not be desirable (or even possible),
// so the application can supply an implementation of PasswordPrompt
// to take responsibility for obtaining the password.
//
// Note that the password is needed during plug-in initialization, so
// in general you will need to delay initialization (by defining
// IceSSL.DelayInit=1), configure the PasswordPrompt, then manually
// initialize the plug-in.
//
class ICE_SSL_API PasswordPrompt : public IceUtil::Shared
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

class ICE_SSL_API Plugin : public Ice::Plugin
{
public:

    //
    // Establish the certificate verifier object. This should be done
    // before any connections are established.
    //
    virtual void setCertificateVerifier(const CertificateVerifierPtr&) = 0;

    //
    // Establish the password prompt object. This must be done before
    // the plug-in is initialized.
    //
    virtual void setPasswordPrompt(const PasswordPromptPtr&) = 0;

#ifdef ICE_USE_OPENSSL
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
#endif
};
typedef IceUtil::Handle<Plugin> PluginPtr;

}

#endif
