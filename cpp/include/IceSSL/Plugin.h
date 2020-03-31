//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_PLUGIN_H
#define ICESSL_PLUGIN_H

#include <Ice/Plugin.h>
#include <IceSSL/Config.h>
#include <IceSSL/ConnectionInfoF.h>
#include <IceSSL/EndpointInfo.h>

#   include <chrono>

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

/**
 * Thrown if the certificate cannot be read.
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_API CertificateReadException : public IceUtil::ExceptionHelper<CertificateReadException>
{
public:

    CertificateReadException(const char*, int, const std::string&);

    virtual std::string ice_id() const;

    /** The reason for the exception. */
    std::string reason;

private:

    static const char* _name;
};

/**
 * Thrown if the certificate cannot be encoded.
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_API CertificateEncodingException : public IceUtil::ExceptionHelper<CertificateEncodingException>
{
public:

    CertificateEncodingException(const char*, int, const std::string&);

    virtual std::string ice_id() const;

    /** The reason for the exception. */
    std::string reason;

private:

    static const char* _name;
};

/**
 * This exception is thrown if a distinguished name cannot be parsed.
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_API ParseException : public IceUtil::ExceptionHelper<ParseException>
{
public:

    ParseException(const char*, int, const std::string&);

    virtual std::string ice_id() const;

    /** The reason for the exception. */
    std::string reason;

private:

    static const char* _name;
};

/**
 * This class represents a DistinguishedName, similar to the Java
 * type X500Principal and the .NET type X500DistinguishedName.
 *
 * For comparison purposes, the value of a relative distinguished
 * name (RDN) component is always unescaped before matching,
 * therefore "ZeroC, Inc." will match ZeroC\, Inc.
 *
 * toString() always returns exactly the same information as was
 * provided in the constructor (i.e., "ZeroC, Inc." will not turn
 * into ZeroC\, Inc.).
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_API DistinguishedName
{
public:

    /**
     * Creates a DistinguishedName from a string encoded using the rules in RFC2253.
     * @param name The encoded distinguished name.
     * @throws ParseException if parsing fails.
     */
    explicit DistinguishedName(const std::string& name);

    /**
     * Creates a DistinguishedName from a list of RDN pairs,
     * where each pair consists of the RDN's type and value.
     * For example, the RDN "O=ZeroC" is represented by the
     * pair ("O", "ZeroC").
     * @throws ParseException if parsing fails.
     */
    explicit DistinguishedName(const std::list<std::pair<std::string, std::string> >&);

    /**
     * Performs an exact match. The order of the RDN components is important.
     */
    friend ICESSL_API bool operator==(const DistinguishedName&, const DistinguishedName&);

    /**
     * Performs an exact match. The order of the RDN components is important.
     */
    friend ICESSL_API bool operator<(const DistinguishedName&, const DistinguishedName&);

    /**
     * Performs a partial match with another DistinguishedName.
     * @param dn The name to be matched.
     * @return True if all of the RDNs in the argument are present in this
     * DistinguishedName and they have the same values.
     */
    bool match(const DistinguishedName& dn) const;

    /**
     * Performs a partial match with another DistinguishedName.
     * @param dn The name to be matched.
     * @return True if all of the RDNs in the argument are present in this
     * DistinguishedName and they have the same values.
     */
    bool match(const std::string& dn) const;

    /**
     * Encodes the DN in RFC2253 format.
     * @return An encoded string.
     */
    operator std::string() const;

protected:

    /// \cond INTERNAL
    void unescape();
    /// \endcond

private:

    std::list<std::pair<std::string, std::string> > _rdns;
    std::list<std::pair<std::string, std::string> > _unescaped;
};

/**
 * Performs an exact match. The order of the RDN components is important.
 */
inline bool
operator>(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return rhs < lhs;
}

/**
 * Performs an exact match. The order of the RDN components is important.
 */
inline bool
operator<=(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return !(lhs > rhs);
}

/**
 * Performs an exact match. The order of the RDN components is important.
 */
inline bool
operator>=(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return !(lhs < rhs);
}

/**
 * Performs an exact match. The order of the RDN components is important.
 */
inline bool
operator!=(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return !(lhs == rhs);
}

/**
 * Represents an X509 Certificate extension.
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_API X509Extension
{
public:

    /**
     * Determines whether the information in this extension is important.
     * @return True if if information is important, false otherwise.
     */
    virtual bool isCritical() const = 0;

    /**
     * Obtains the object ID of this extension.
     * @return The object ID.
     */
    virtual std::string getOID() const = 0;

    /**
     * Obtains the data associated with this extension.
     * @return The extension data.
     */
    virtual std::vector<Ice::Byte> getData() const = 0;
};
ICE_DEFINE_PTR(X509ExtensionPtr, X509Extension);

class Certificate;
ICE_DEFINE_PTR(CertificatePtr, Certificate);

/**
 * This convenience class is a wrapper around a native certificate.
 * The interface is inspired by java.security.cert.X509Certificate.
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_API Certificate :
    public std::enable_shared_from_this<Certificate>
{
public:

    /**
     * Compares the certificates for equality using the native certificate comparison method.
     */
    virtual bool operator==(const Certificate&) const = 0;

    /**
     * Compares the certificates for equality using the native certificate comparison method.
     */
    virtual bool operator!=(const Certificate&) const = 0;

    /**
     * Obtains the authority key identifier.
     * @return The identifier.
     */
    virtual std::vector<Ice::Byte> getAuthorityKeyIdentifier() const = 0;

    /**
     * Obtains the subject key identifier.
     * @return The identifier.
     */
    virtual std::vector<Ice::Byte> getSubjectKeyIdentifier() const = 0;

    /**
     * Verifies that this certificate was signed by the given certificate
     * public key.
     * @param cert A certificate containing the public key.
     * @return True if signed, false otherwise.
     */
    virtual bool verify(const CertificatePtr& cert) const = 0;

    /**
     * Obtains a string encoding of the certificate in PEM format.
     * @return The encoded certificate.
     * @throws CertificateEncodingException if an error occurs.
     */
    virtual std::string encode() const = 0;

    /**
     * Checks that the certificate is currently valid, that is, the current
     * date falls between the validity period given in the certificate.
     * @return True if the certificate is valid, false otherwise.
     */
    virtual bool checkValidity() const = 0;

    /**
     * Checks that the certificate is valid at the given time.
     * @param t The target time.
     * @return True if the certificate is valid, false otherwise.
     */
    virtual bool checkValidity(const std::chrono::system_clock::time_point& t) const = 0;

    /**
     * Obtains the not-after validity time.
     * @return The time after which this certificate is invalid.
     */
    virtual std::chrono::system_clock::time_point getNotAfter() const = 0;

    /**
     * Obtains the not-before validity time.
     * @return The time at which this certificate is valid.
     */
    virtual std::chrono::system_clock::time_point getNotBefore() const = 0;

    /**
     * Obtains the serial number. This is an arbitrarily large number.
     * @return The certificate's serial number.
     */
    virtual std::string getSerialNumber() const = 0;

    /**
     * Obtains the issuer's distinguished name (DN).
     * @return The distinguished name.
     */
    virtual DistinguishedName getIssuerDN() const = 0;

    /**
     * Obtains the values in the issuer's alternative names extension.
     *
     * The returned list contains a pair of int, string.
     *
     * otherName                       [0]     OtherName
     * rfc822Name                      [1]     IA5String
     * dNSName                         [2]     IA5String
     * x400Address                     [3]     ORAddress
     * directoryName                   [4]     Name
     * ediPartyName                    [5]     EDIPartyName
     * uniformResourceIdentifier       [6]     IA5String
     * iPAddress                       [7]     OCTET STRING
     * registeredID                    [8]     OBJECT IDENTIFIER
     *
     * rfc822Name, dNSName, directoryName and
     * uniformResourceIdentifier data is returned as a string.
     *
     * iPAddress is returned in dotted quad notation. IPv6 is not
     * currently supported.
     *
     * All distinguished names are encoded in RFC2253 format.
     *
     * The remainder of the data will result in an empty string. Use the raw
     * X509* certificate to obtain these values.
     *
     * @return The issuer's alternative names.
     */
    virtual std::vector<std::pair<int, std::string> > getIssuerAlternativeNames() const = 0;

    /**
     * Obtains the subject's distinguished name (DN).
     * @return The distinguished name.
     */
    virtual DistinguishedName getSubjectDN() const = 0;

    /**
     * See the comment for Plugin::getIssuerAlternativeNames.
     * @return The subject's alternative names.
     */
    virtual std::vector<std::pair<int, std::string> > getSubjectAlternativeNames() const = 0;

    /**
     * Obtains the certificate version number.
     * @return The version number.
     */
    virtual int getVersion() const = 0;

    /**
     * Stringifies the certificate. This is a human readable version of
     * the certificate, not a DER or PEM encoding.
     * @return A string version of the certificate.
     */
    virtual std::string toString() const = 0;

    /**
     * Obtains a list of the X509v3 extensions contained in the certificate.
     * @return The extensions.
     */
    virtual std::vector<X509ExtensionPtr> getX509Extensions() const = 0;

    /**
     * Obtains the extension with the given OID.
     * @return The extension, or null if the certificate
     * does not contain a extension with the given OID.
     */
    virtual X509ExtensionPtr getX509Extension(const std::string& oid) const = 0;

    /**
     * Loads the certificate from a file. The certificate must use the
     * PEM encoding format.
     * @param file The certificate file.
     * @return The new certificate instance.
     * @throws CertificateReadException if the file cannot be read.
     */
    static CertificatePtr load(const std::string& file);

    /**
     * Decodes a certificate from a string that uses the PEM encoding format.
     * @param str A string containing the encoded certificate.
     * @throws CertificateEncodingException if an error occurs.
     */
    static CertificatePtr decode(const std::string& str);
};

/**
 * Represents the IceSSL plug-in object.
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_API Plugin : public Ice::Plugin
{
public:

    virtual ~Plugin();

    /**
     * Establish the certificate verifier object. This should be done
     * before any connections are established.
     * @param v The verifier.
     */
    virtual void setCertificateVerifier(std::function<bool(const std::shared_ptr<ConnectionInfo>&)> v) = 0;

    /**
     * Establish the password prompt object. This must be done before
     * the plug-in is initialized.
     * @param p The password prompt.
     */
    virtual void setPasswordPrompt(std::function<std::string()> p) = 0;

    /**
     * Load the certificate from a file. The certificate must use the
     * PEM encoding format.
     * @param file The certificate file.
     * @throws CertificateReadException if the file cannot be read.
     */
    virtual CertificatePtr load(const std::string& file) const = 0;

    /**
     * Decode a certificate from a string that uses the PEM encoding
     * format.
     * @param str A string containing the encoded certificate.
     * @throws CertificateEncodingException if an error occurs.
     */
    virtual CertificatePtr decode(const std::string& str) const = 0;
};
ICE_DEFINE_PTR(PluginPtr, Plugin);

}

#endif
