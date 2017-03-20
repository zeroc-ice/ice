// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_PLUGIN_H
#define ICESSL_PLUGIN_H

#include <Ice/Plugin.h>
#include <IceSSL/Config.h>
#include <IceSSL/ConnectionInfoF.h>

#ifdef ICE_CPP11_MAPPING
#   include <chrono>
#else
#   include <IceUtil/Time.h>
#endif

#include <vector>
#include <list>

#ifndef ICESSL_API
#   if defined(ICE_STATIC_LIBS)
#       define ICESSL_API /**/
#   elif defined(ICESSL_API_EXPORTS)
#       define ICESSL_API ICE_DECLSPEC_EXPORT
#   else
#       define ICESSL_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceSSL
{

//
// This exception is thrown if the certificate cannot be read.
//
class ICESSL_API CertificateReadException : public IceUtil::ExceptionHelper<CertificateReadException>
{
public:

    CertificateReadException(const char*, int, const std::string&);
#ifndef ICE_CPP11_COMPILER
    virtual ~CertificateReadException() throw();
#endif
    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual CertificateReadException* ice_clone() const;
#endif

    std::string reason;

private:

    static const char* _name;
};

//
// This exception is thrown if the certificate cannot be encoded.
//
class ICESSL_API CertificateEncodingException : public IceUtil::ExceptionHelper<CertificateEncodingException>
{
public:

    CertificateEncodingException(const char*, int, const std::string&);
#ifndef ICE_CPP11_COMPILER
    virtual ~CertificateEncodingException() throw();
#endif
    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual CertificateEncodingException* ice_clone() const;
#endif

    std::string reason;

private:

    static const char* _name;
};

//
// This exception is thrown if a distinguished name cannot be parsed.
//
class ICESSL_API ParseException : public IceUtil::ExceptionHelper<ParseException>
{
public:

    ParseException(const char*, int, const std::string&);
#ifndef ICE_CPP11_COMPILER
    virtual ~ParseException() throw();
#endif
    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual ParseException* ice_clone() const;
#endif

    std::string reason;

private:

    static const char* _name;
};

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
class ICESSL_API DistinguishedName
{
public:

    //
    // Create a DistinguishedName from a string encoded using
    // the rules in RFC2253.
    //
    // Throws ParseException if parsing fails.
    //
    explicit DistinguishedName(const std::string&);

    //
    // Create a DistinguishedName from a list of RDN pairs,
    // where each pair consists of the RDN's type and value.
    // For example, the RDN "O=ZeroC" is represented by the
    // pair ("O", "ZeroC").
    //
    explicit DistinguishedName(const std::list<std::pair<std::string, std::string> >&);

    //
    // This is an exact match. The order of the RDN components is
    // important.
    //
    friend ICESSL_API bool operator==(const DistinguishedName&, const DistinguishedName&);
    friend ICESSL_API bool operator<(const DistinguishedName&, const DistinguishedName&);

    //
    // Perform a partial match with another DistinguishedName. The function
    // returns true if all of the RDNs in the argument are present in this
    // DistinguishedName and they have the same values.
    //
    bool match(const DistinguishedName&) const;
    bool match(const std::string&) const;

    //
    // Encode the DN in RFC2253 format.
    //
    operator std::string() const;

protected:

	void unescape();

private:

    std::list<std::pair<std::string, std::string> > _rdns;
    std::list<std::pair<std::string, std::string> > _unescaped;
};

inline bool
operator>(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return rhs < lhs;
}

inline bool
operator<=(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return !(lhs > rhs);
}

inline bool
operator>=(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return !(lhs < rhs);
}

inline bool
operator!=(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return !(lhs == rhs);
}

//
// This class represents an X509 Certificate extension.
//
class ICESSL_API X509Extension
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual bool isCritical() const = 0;
    virtual std::string getOID() const = 0;
    virtual std::vector<Ice::Byte> getData() const = 0;
};
ICE_DEFINE_PTR(X509ExtensionPtr, X509Extension);

//
// This convenience class is a wrapper around a native certificate.
// The interface is inspired by java.security.cert.X509Certificate.
//

class Certificate;
ICE_DEFINE_PTR(CertificatePtr, Certificate);

class ICESSL_API Certificate :
#ifdef ICE_CPP11_MAPPING
        public std::enable_shared_from_this<Certificate>
#else
        public virtual IceUtil::Shared
#endif
{
public:

    //
    // Compare the certificates for equality using the
    // native certificate comparison method.
    //
    virtual bool operator==(const Certificate&) const = 0;
    virtual bool operator!=(const Certificate&) const = 0;
    
    //
    // Authority key identifier
    //
    virtual std::vector<Ice::Byte> getAuthorityKeyIdentifier() const = 0;

    //
    // Subject key identifier
    //
    virtual std::vector<Ice::Byte> getSubjectKeyIdentifier() const = 0;

    //
    // Verify that this certificate was signed by the given certificate
    // public key. Returns true if signed, false otherwise.
    //
    virtual bool verify(const CertificatePtr&) const = 0;

    //
    // Return a string encoding of the certificate in PEM format.
    // Raises CertificateEncodingException if an error occurs.
    //
    virtual std::string encode() const = 0;

    //
    // Checks that the certificate is currently valid, that is, the current
    // date falls between the validity period given in the certificate.
    //
    virtual bool checkValidity() const = 0;

    //
    // Checks that the certificate is valid at the given time.
    //
#ifdef ICE_CPP11_MAPPING
    virtual bool checkValidity(const std::chrono::system_clock::time_point&) const = 0;
#else
    virtual bool checkValidity(const IceUtil::Time&) const = 0;
#endif

    //
    // Get the not-after validity time.
    //
#ifdef ICE_CPP11_MAPPING
    virtual std::chrono::system_clock::time_point getNotAfter() const = 0;
#else
    virtual IceUtil::Time getNotAfter() const = 0;
#endif

    //
    // Get the not-before validity time.
    //
#ifdef ICE_CPP11_MAPPING
    virtual std::chrono::system_clock::time_point getNotBefore() const = 0;
#else
    virtual IceUtil::Time getNotBefore() const = 0;
#endif

    //
    // Get the serial number. This is an arbitrarily large number.
    //
    virtual std::string getSerialNumber() const = 0;

    //
    // Get the issuer's distinguished name (DN).
    //
    virtual DistinguishedName getIssuerDN() const = 0;

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
    virtual std::vector<std::pair<int, std::string> > getIssuerAlternativeNames() const = 0;

    //
    // Get the subject's distinguished name (DN).
    //
    virtual DistinguishedName getSubjectDN() const = 0;

    //
    // See the comment for getIssuerAlternativeNames.
    //
    virtual std::vector<std::pair<int, std::string> > getSubjectAlternativeNames() const = 0;

    //
    // Retrieve the certificate version number.
    //
    virtual int getVersion() const = 0;

    //
    // Stringify the certificate. This is a human readable version of
    // the certificate, not a DER or PEM encoding.
    //
    virtual std::string toString() const = 0;

    //
    // Return a list with the X509v3 extensions contained in the 
    // certificate.
    //
    virtual std::vector<X509ExtensionPtr> getX509Extensions() const = 0;
    
    //
    // Return the extension with the given OID or null if the certificate
    // does not contain a extension with the given OID.
    //
    virtual X509ExtensionPtr getX509Extension(const std::string&) const = 0;

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
};


#ifndef ICE_CPP11_MAPPING // C++98 mapping
//
// An application can customize the certificate verification process
// by implementing the CertificateVerifier interface.
//

class ICESSL_API CertificateVerifier : public IceUtil::Shared
{
public:

    virtual ~CertificateVerifier();

    //
    // Return false if the connection should be rejected, or true to
    // allow it.
    //
    virtual bool verify(const ConnectionInfoPtr&) = 0;
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
class ICESSL_API PasswordPrompt : public IceUtil::Shared
{
public:

    virtual ~PasswordPrompt();

    //
    // The getPassword method may be invoked repeatedly, such as when
    // several encrypted files are opened, or when multiple password
    // attempts are allowed.
    //
    virtual std::string getPassword() = 0;
};
typedef IceUtil::Handle<PasswordPrompt> PasswordPromptPtr;
#endif

class ICESSL_API Plugin : public Ice::Plugin
{
public:

    virtual ~Plugin();
    
    virtual std::string getEngineName() const = 0;
    virtual Ice::Long getEngineVersion() const = 0;

    //
    // Establish the certificate verifier object. This should be done
    // before any connections are established.
    //
#ifdef ICE_CPP11_MAPPING
    virtual void setCertificateVerifier(std::function<bool(const std::shared_ptr<ConnectionInfo>&)>) = 0;
#else
    virtual void setCertificateVerifier(const CertificateVerifierPtr&) = 0;
#endif

    //
    // Establish the password prompt object. This must be done before
    // the plug-in is initialized.
    //
#ifdef ICE_CPP11_MAPPING
    virtual void setPasswordPrompt(std::function<std::string()>) = 0;
#else
    virtual void setPasswordPrompt(const PasswordPromptPtr&) = 0;
#endif

    //
    // Load the certificate from a file. The certificate must use the
    // PEM encoding format. Raises CertificateReadException if the
    // file cannot be read.
    //
    virtual CertificatePtr load(const std::string&) const = 0;

    //
    // Decode a certificate from a string that uses the PEM encoding
    // format. Raises CertificateEncodingException if an error
    // occurs.
    //
    virtual CertificatePtr decode(const std::string&) const = 0;
};
ICE_DEFINE_PTR(PluginPtr, Plugin);

}

#endif
