//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "PluginI.h"
#include "IceSSL/OpenSSL.h"
#include "CertificateI.h"
#include "OpenSSLUtil.h"
#include "RFC2253.h"

#include <cassert>
#include <mutex>
#include <sstream>
#include <stdexcept>

#include <openssl/x509v3.h>
#include <openssl/pem.h>

using namespace IceSSL;
using namespace std;

//
// Avoid old style cast warnings from OpenSSL macros
//
#if defined(__GNUC__)
#    pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#if defined(OPENSSL_VERSION_NUMBER) && OPENSSL_VERSION_NUMBER < 0x10100000L
#    define X509_get_extension_flags(x) (x->ex_flags)
#    define X509_get_key_usage(x) (x->ex_kusage)
#    define X509_get_extended_key_usage(x) (x->ex_xkusage)
#endif

namespace
{
    static string convertX509NameToString(X509_name_st* name)
    {
        BIO* out = BIO_new(BIO_s_mem());
        X509_NAME_print_ex(out, name, 0, XN_FLAG_RFC2253);
        BUF_MEM* p;
        BIO_get_mem_ptr(out, &p);
        string result = string(p->data, p->length);
        BIO_free(out);
        return result;
    }

    static vector<pair<int, string>> convertGeneralNames(GENERAL_NAMES* gens)
    {
        vector<pair<int, string>> alt;
        if (gens == 0)
        {
            return alt;
        }
        for (int i = 0; i < sk_GENERAL_NAME_num(gens); ++i)
        {
            GENERAL_NAME* gen = sk_GENERAL_NAME_value(gens, i);
            pair<int, string> p;
            p.first = gen->type;
            switch (gen->type)
            {
                case GEN_EMAIL:
                {
                    ASN1_IA5STRING* str = gen->d.rfc822Name;
                    if (str && str->type == V_ASN1_IA5STRING && str->data && str->length > 0)
                    {
                        p.second = string(reinterpret_cast<const char*>(str->data), str->length);
                    }
                    break;
                }
                case GEN_DNS:
                {
                    ASN1_IA5STRING* str = gen->d.dNSName;
                    if (str && str->type == V_ASN1_IA5STRING && str->data && str->length > 0)
                    {
                        p.second = string(reinterpret_cast<const char*>(str->data), str->length);
                    }
                    break;
                }
                case GEN_DIRNAME:
                {
                    p.second = convertX509NameToString(gen->d.directoryName);
                    break;
                }
                case GEN_URI:
                {
                    ASN1_IA5STRING* str = gen->d.uniformResourceIdentifier;
                    if (str && str->type == V_ASN1_IA5STRING && str->data && str->length > 0)
                    {
                        p.second = string(reinterpret_cast<const char*>(str->data), str->length);
                    }
                    break;
                }
                case GEN_IPADD:
                {
                    ASN1_OCTET_STRING* addr = gen->d.iPAddress;
                    // TODO: Support IPv6 someday.
                    if (addr && addr->type == V_ASN1_OCTET_STRING && addr->data && addr->length == 4)
                    {
                        ostringstream ostr;
                        for (int j = 0; j < 4; ++j)
                        {
                            if (j > 0)
                            {
                                ostr << '.';
                            }
                            ostr << static_cast<int>(addr->data[j]);
                        }
                        p.second = ostr.str();
                    }
                    break;
                }
                case GEN_OTHERNAME:
                case GEN_EDIPARTY:
                case GEN_X400:
                case GEN_RID:
                {
                    //
                    // TODO: These types are not supported. If the user wants
                    // them, they have to get at the certificate data. Another
                    // alternative is to DER encode the data (as the Java
                    // certificate does).
                    //
                    break;
                }
            }
            if (!p.second.empty())
            {
                alt.push_back(p);
            }
        }
        sk_GENERAL_NAME_pop_free(gens, GENERAL_NAME_free);
        return alt;
    }

    class DistinguishedNameI : public IceSSL::DistinguishedName
    {
    public:
        DistinguishedNameI(X509_name_st* name)
            : IceSSL::DistinguishedName(IceSSL::RFC2253::parseStrict(convertX509NameToString(name)))
        {
            unescape();
        }
    };

    mutex globalMutex;

    chrono::system_clock::time_point ASMUtcTimeToTime(const ASN1_UTCTIME* s)
    {
        struct tm tm;
        int offset;

        memset(&tm, '\0', sizeof tm);

#define g2(p) (((p)[0] - '0') * 10 + (p)[1] - '0')
        tm.tm_year = g2(s->data);
        if (tm.tm_year < 50)
        {
            tm.tm_year += 100;
        }
        tm.tm_mon = g2(s->data + 2) - 1;
        tm.tm_mday = g2(s->data + 4);
        tm.tm_hour = g2(s->data + 6);
        tm.tm_min = g2(s->data + 8);
        tm.tm_sec = g2(s->data + 10);
        if (s->data[12] == 'Z')
        {
            offset = 0;
        }
        else
        {
            offset = g2(s->data + 13) * 60 + g2(s->data + 15);
            if (s->data[12] == '-')
            {
                offset = -offset;
            }
        }
#undef g2

        time_t tzone;
        {
            lock_guard lock(globalMutex);
            time_t now = time(0);
            struct tm localTime;
            struct tm gmTime;
#if defined(_MSC_VER)
            localtime_s(&localTime, &now);
            gmtime_s(&gmTime, &now);
#else
            localtime_r(&now, &localTime);
            gmtime_r(&now, &gmTime);
#endif
            tzone = mktime(&localTime) - mktime(&gmTime);
        }
        return chrono::system_clock::time_point(chrono::seconds(mktime(&tm) - int64_t{offset} * 60 + tzone));
    }

    class OpenSSLX509ExtensionI : public IceSSL::X509Extension
    {
    public:
        OpenSSLX509ExtensionI(struct X509_extension_st*, const string&, x509_st*);
        ~OpenSSLX509ExtensionI();
        virtual bool isCritical() const;
        virtual string getOID() const;
        virtual vector<uint8_t> getData() const;

    private:
        struct X509_extension_st* _extension;
        string _oid;
        x509_st* _cert;
    };

    class OpenSSLCertificateI : public IceSSL::OpenSSL::Certificate,
                                public CertificateI,
                                public IceSSL::CertificateExtendedInfo
    {
    public:
        OpenSSLCertificateI(x509_st*);
        ~OpenSSLCertificateI();

        virtual bool operator==(const IceSSL::Certificate&) const;

        virtual vector<uint8_t> getAuthorityKeyIdentifier() const;
        virtual vector<uint8_t> getSubjectKeyIdentifier() const;
        virtual bool verify(const IceSSL::CertificatePtr&) const;
        virtual string encode() const;

        virtual chrono::system_clock::time_point getNotAfter() const;
        virtual chrono::system_clock::time_point getNotBefore() const;
        virtual string getSerialNumber() const;
        virtual IceSSL::DistinguishedName getIssuerDN() const;
        virtual vector<pair<int, string>> getIssuerAlternativeNames() const;
        virtual IceSSL::DistinguishedName getSubjectDN() const;
        virtual vector<pair<int, string>> getSubjectAlternativeNames() const;
        virtual int getVersion() const;
        virtual x509_st* getCert() const;
        virtual unsigned int getKeyUsage() const;
        virtual unsigned int getExtendedKeyUsage() const;

    protected:
        virtual void loadX509Extensions() const;

    private:
        x509_st* _cert;
        mutable std::mutex _mutex;
    };

} // end anonymous namespace

OpenSSLX509ExtensionI::OpenSSLX509ExtensionI(struct X509_extension_st* extension, const string& oid, x509_st* cert)
    : _extension(extension),
      _oid(oid),
      _cert(cert)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
    CRYPTO_add(&_cert->references, 1, CRYPTO_LOCK_X509);
#else
    X509_up_ref(_cert);
#endif
}

OpenSSLX509ExtensionI::~OpenSSLX509ExtensionI() { X509_free(_cert); }

bool
OpenSSLX509ExtensionI::isCritical() const
{
    return X509_EXTENSION_get_critical(_extension) == 1;
}

string
OpenSSLX509ExtensionI::getOID() const
{
    return _oid;
}

vector<uint8_t>
OpenSSLX509ExtensionI::getData() const
{
    vector<uint8_t> data;
    ASN1_OCTET_STRING* buffer = X509_EXTENSION_get_data(_extension);
    assert(buffer);
    data.resize(buffer->length);
    memcpy(&data[0], buffer->data, buffer->length);
    return data;
}

//
// The caller is responsible for incrementing the reference count.
//
OpenSSLCertificateI::OpenSSLCertificateI(x509_st* cert) : _cert(cert)
{
    if (!_cert)
    {
        throw invalid_argument("Invalid certificate reference");
    }
}

OpenSSLCertificateI::~OpenSSLCertificateI()
{
    if (_cert)
    {
        X509_free(_cert);
    }
}

bool
OpenSSLCertificateI::operator==(const IceSSL::Certificate& r) const
{
    const OpenSSLCertificateI* p = dynamic_cast<const OpenSSLCertificateI*>(&r);
    if (!p)
    {
        return false;
    }

    return X509_cmp(_cert, p->_cert) == 0;
}

vector<uint8_t>
OpenSSLCertificateI::getAuthorityKeyIdentifier() const
{
    vector<uint8_t> keyid;
    int index = X509_get_ext_by_NID(_cert, NID_authority_key_identifier, -1);
    if (index >= 0)
    {
        X509_EXTENSION* ext = X509_get_ext(_cert, index);
        if (ext)
        {
            AUTHORITY_KEYID* decoded = (AUTHORITY_KEYID*)X509V3_EXT_d2i(ext);
            if (!decoded)
            {
                throw IceSSL::CertificateEncodingException(__FILE__, __LINE__, "the extension could not be decoded");
            }
            keyid.resize(decoded->keyid->length);
            memcpy(&keyid[0], decoded->keyid->data, decoded->keyid->length);
            AUTHORITY_KEYID_free(decoded);
        }
    }
    return keyid;
}

vector<uint8_t>
OpenSSLCertificateI::getSubjectKeyIdentifier() const
{
    vector<uint8_t> keyid;
    int index = X509_get_ext_by_NID(_cert, NID_subject_key_identifier, -1);
    if (index >= 0)
    {
        X509_EXTENSION* ext = X509_get_ext(_cert, index);
        if (ext)
        {
            ASN1_OCTET_STRING* decoded = static_cast<ASN1_OCTET_STRING*>(X509V3_EXT_d2i(ext));
            if (!decoded)
            {
                throw IceSSL::CertificateEncodingException(__FILE__, __LINE__, "the extension could not be decoded");
            }
            keyid.resize(decoded->length);
            memcpy(&keyid[0], decoded->data, decoded->length);
            ASN1_OCTET_STRING_free(decoded);
        }
    }
    return keyid;
}

bool
OpenSSLCertificateI::verify(const IceSSL::CertificatePtr& cert) const
{
    OpenSSLCertificateI* c = dynamic_cast<OpenSSLCertificateI*>(cert.get());
    if (c)
    {
        EVP_PKEY* key = X509_get_pubkey(c->_cert);
        bool verified = X509_verify(_cert, key) > 0;
        EVP_PKEY_free(key);
        return verified;
    }
    return false;
}

string
OpenSSLCertificateI::encode() const
{
    BIO* out = BIO_new(BIO_s_mem());
    int i = PEM_write_bio_X509(out, _cert);
    if (i <= 0)
    {
        BIO_free(out);
        throw IceSSL::CertificateEncodingException(__FILE__, __LINE__, IceSSL::OpenSSL::getSslErrors(false));
    }
    BUF_MEM* p;
    BIO_get_mem_ptr(out, &p);
    string result = string(p->data, p->length);
    BIO_free(out);
    return result;
}

chrono::system_clock::time_point
OpenSSLCertificateI::getNotAfter() const
{
    return ASMUtcTimeToTime(X509_get_notAfter(_cert));
}

chrono::system_clock::time_point
OpenSSLCertificateI::getNotBefore() const
{
    return ASMUtcTimeToTime(X509_get_notBefore(_cert));
}

string
OpenSSLCertificateI::getSerialNumber() const
{
    BIGNUM* bn = ASN1_INTEGER_to_BN(X509_get_serialNumber(_cert), 0);
    char* dec = BN_bn2dec(bn);
    string result = dec;
    OPENSSL_free(dec);
    BN_free(bn);
    return result;
}

IceSSL::DistinguishedName
OpenSSLCertificateI::getIssuerDN() const
{
    return IceSSL::DistinguishedName(
        IceSSL::RFC2253::parseStrict(convertX509NameToString(X509_get_issuer_name(_cert))));
}

vector<pair<int, string>>
OpenSSLCertificateI::getIssuerAlternativeNames() const
{
    return convertGeneralNames(reinterpret_cast<GENERAL_NAMES*>(X509_get_ext_d2i(_cert, NID_issuer_alt_name, 0, 0)));
}

IceSSL::DistinguishedName
OpenSSLCertificateI::getSubjectDN() const
{
    return IceSSL::DistinguishedName(
        IceSSL::RFC2253::parseStrict(convertX509NameToString(X509_get_subject_name(_cert))));
}

vector<pair<int, string>>
OpenSSLCertificateI::getSubjectAlternativeNames() const
{
    return convertGeneralNames(reinterpret_cast<GENERAL_NAMES*>(X509_get_ext_d2i(_cert, NID_subject_alt_name, 0, 0)));
}

int
OpenSSLCertificateI::getVersion() const
{
    return static_cast<int>(X509_get_version(_cert));
}

x509_st*
OpenSSLCertificateI::getCert() const
{
    return _cert;
}

void
OpenSSLCertificateI::loadX509Extensions() const
{
    lock_guard lock(_mutex);
    if (_extensions.empty())
    {
        int sz = X509_get_ext_count(_cert);
        for (int i = 0; i < sz; i++)
        {
            X509_EXTENSION* ext = X509_get_ext(_cert, i);
            ASN1_OBJECT* obj = X509_EXTENSION_get_object(ext);
            string oid;
            //
            // According to OBJ_obj2txt doc a buffer of length 80 should be more than enough to
            // handle any OID encountered in practice.
            //
            int len = 80;
            oid.resize(len);
            len = OBJ_obj2txt(&oid[0], len, obj, 1);
            oid.resize(len);
            _extensions.push_back(
                dynamic_pointer_cast<IceSSL::X509Extension>(make_shared<OpenSSLX509ExtensionI>(ext, oid, _cert)));
        }
    }
}

unsigned int
OpenSSLCertificateI::getKeyUsage() const
{
    unsigned int keyUsage = 0;
    int flags = X509_get_extension_flags(_cert);
    if (flags & EXFLAG_KUSAGE)
    {
        unsigned int kusage = X509_get_key_usage(_cert);
        if (kusage & KU_DIGITAL_SIGNATURE)
        {
            keyUsage |= KEY_USAGE_DIGITAL_SIGNATURE;
        }
        if (kusage & KU_NON_REPUDIATION)
        {
            keyUsage |= KEY_USAGE_NON_REPUDIATION;
        }
        if (kusage & KU_KEY_ENCIPHERMENT)
        {
            keyUsage |= KEY_USAGE_KEY_ENCIPHERMENT;
        }
        if (kusage & KU_DATA_ENCIPHERMENT)
        {
            keyUsage |= KEY_USAGE_DATA_ENCIPHERMENT;
        }
        if (kusage & KU_KEY_AGREEMENT)
        {
            keyUsage |= KEY_USAGE_KEY_AGREEMENT;
        }
        if (kusage & KU_KEY_CERT_SIGN)
        {
            keyUsage |= KEY_USAGE_KEY_CERT_SIGN;
        }
        if (kusage & KU_CRL_SIGN)
        {
            keyUsage |= KEY_USAGE_CRL_SIGN;
        }
        if (kusage & KU_ENCIPHER_ONLY)
        {
            keyUsage |= KEY_USAGE_ENCIPHER_ONLY;
        }
        if (kusage & KU_DECIPHER_ONLY)
        {
            keyUsage |= KEY_USAGE_DECIPHER_ONLY;
        }
    }
    return keyUsage;
}

unsigned int
OpenSSLCertificateI::getExtendedKeyUsage() const
{
    unsigned int extendedKeyUsage = 0;
    int flags = X509_get_extension_flags(_cert);
    if (flags & EXFLAG_XKUSAGE)
    {
        unsigned int xkusage = X509_get_extended_key_usage(_cert);
        if (xkusage & XKU_ANYEKU)
        {
            extendedKeyUsage |= EXTENDED_KEY_USAGE_ANY_KEY_USAGE;
        }
        if (xkusage & XKU_SSL_SERVER)
        {
            extendedKeyUsage |= EXTENDED_KEY_USAGE_SERVER_AUTH;
        }
        if (xkusage & XKU_SSL_CLIENT)
        {
            extendedKeyUsage |= EXTENDED_KEY_USAGE_CLIENT_AUTH;
        }
        if (xkusage & XKU_CODE_SIGN)
        {
            extendedKeyUsage |= EXTENDED_KEY_USAGE_CODE_SIGNING;
        }
        if (xkusage & XKU_SMIME)
        {
            extendedKeyUsage |= EXTENDED_KEY_USAGE_EMAIL_PROTECTION;
        }
        if (xkusage & XKU_TIMESTAMP)
        {
            extendedKeyUsage |= EXTENDED_KEY_USAGE_TIME_STAMPING;
        }
        if (xkusage & XKU_OCSP_SIGN)
        {
            extendedKeyUsage |= EXTENDED_KEY_USAGE_OCSP_SIGNING;
        }
    }
    return extendedKeyUsage;
}

IceSSL::OpenSSL::CertificatePtr
IceSSL::OpenSSL::Certificate::create(x509_st* cert)
{
    return make_shared<OpenSSLCertificateI>(cert);
}

IceSSL::OpenSSL::CertificatePtr
IceSSL::OpenSSL::Certificate::load(const std::string& file)
{
    BIO* cert = BIO_new(BIO_s_file());
    if (BIO_read_filename(cert, file.c_str()) <= 0)
    {
        BIO_free(cert);
        throw CertificateReadException(__FILE__, __LINE__, "error opening file");
    }

    x509_st* x = PEM_read_bio_X509(cert, nullptr, nullptr, nullptr);
    BIO_free(cert);
    if (x == nullptr)
    {
        throw CertificateReadException(__FILE__, __LINE__, "error reading file:\n" + getSslErrors(false));
    }
    // Calling it with -1 for the side effects, this ensure that the extensions info is loaded
    if (X509_check_purpose(x, -1, -1) == -1)
    {
        throw CertificateReadException(__FILE__, __LINE__, "error loading certificate:\n" + getSslErrors(false));
    }
    return make_shared<OpenSSLCertificateI>(x);
}

IceSSL::OpenSSL::CertificatePtr
IceSSL::OpenSSL::Certificate::decode(const std::string& encoding)
{
    BIO* cert = BIO_new_mem_buf(static_cast<void*>(const_cast<char*>(&encoding[0])), static_cast<int>(encoding.size()));
    x509_st* x = PEM_read_bio_X509(cert, nullptr, nullptr, nullptr);
    BIO_free(cert);
    if (x == nullptr)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, getSslErrors(false));
    }
    // Calling it with -1 for the side effects, this ensure that the extensions info is loaded
    if (X509_check_purpose(x, -1, -1) == -1)
    {
        throw CertificateReadException(__FILE__, __LINE__, "error loading certificate:\n" + getSslErrors(false));
    }
    return make_shared<OpenSSLCertificateI>(x);
}
