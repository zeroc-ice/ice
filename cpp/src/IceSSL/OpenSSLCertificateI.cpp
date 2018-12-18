// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/Plugin.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/CertificateI.h>
#include <IceSSL/OpenSSLUtil.h>
#include <IceSSL/RFC2253.h>

#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

#include <openssl/x509v3.h>
#include <openssl/pem.h>

using namespace IceSSL;
using namespace std;

//
// Avoid old style cast warnings from OpenSSL macros
//
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#ifdef __SUNPRO_CC

//
// The call to sk_GENERAL_NAME_pop_free fails to compile if we don't
// remove the extern "C" vs non extern "C" check with the macro below:
//

extern "C" typedef void (*FreeFunc)(void*);

#undef CHECKED_SK_FREE_FUNC
#define CHECKED_SK_FREE_FUNC(type, p) \
    (FreeFunc) (p)

#endif

namespace
{

static string
convertX509NameToString(X509_name_st* name)
{
    BIO* out = BIO_new(BIO_s_mem());
    X509_NAME_print_ex(out, name, 0, XN_FLAG_RFC2253);
    BUF_MEM* p;
    BIO_get_mem_ptr(out, &p);
    string result = string(p->data, p->length);
    BIO_free(out);
    return result;
}

static vector<pair<int, string> >
convertGeneralNames(GENERAL_NAMES* gens)
{
    vector<pair<int, string> > alt;
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
        alt.push_back(p);
    }
    sk_GENERAL_NAME_pop_free(gens, GENERAL_NAME_free);
    return alt;
}

class DistinguishedNameI : public IceSSL::DistinguishedName
{
public:

    DistinguishedNameI(X509_name_st* name) :
        IceSSL::DistinguishedName(IceSSL::RFC2253::parseStrict(convertX509NameToString(name)))
    {
        unescape();
    }

};

IceUtil::Mutex* mut = 0;

class Init
{
public:

    Init()
    {
        mut = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete mut;
        mut = 0;
    }
};

Init init;

#ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#else
static IceUtil::Time
#endif
ASMUtcTimeToTime(const ASN1_UTCTIME* s)
{
    struct tm tm;
    int offset;

    memset(&tm, '\0', sizeof tm);

#  define g2(p) (((p)[0]-'0')*10+(p)[1]-'0')
    tm.tm_year = g2(s->data);
    if(tm.tm_year < 50)
    {
        tm.tm_year += 100;
    }
    tm.tm_mon = g2(s->data + 2) - 1;
    tm.tm_mday = g2(s->data + 4);
    tm.tm_hour = g2(s->data + 6);
    tm.tm_min = g2(s->data + 8);
    tm.tm_sec = g2(s->data + 10);
    if(s->data[12] == 'Z')
    {
        offset = 0;
    }
    else
    {
        offset = g2(s->data + 13) * 60 + g2(s->data + 15);
        if(s->data[12] == '-')
        {
            offset = -offset;
        }
    }
#  undef g2

    //
    // If timegm was on all systems this code could be
    // return IceUtil::Time::seconds(timegm(&tm) - offset*60);
    //
    // Windows doesn't support the re-entrant _r versions.
    //
#if defined(_MSC_VER)
#   pragma warning(disable:4996) // localtime is depercated
#endif
    time_t tzone;
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(mut);
        time_t now = time(0);
        tzone = mktime(localtime(&now)) - mktime(gmtime(&now));
    }
#if defined(_MSC_VER)
#   pragma warning(default:4996) // localtime is depercated
#endif

    IceUtil::Time time = IceUtil::Time::seconds(mktime(&tm) - offset * 60 + tzone);

#ifdef ICE_CPP11_MAPPING
    return chrono::system_clock::time_point(chrono::microseconds(time.toMicroSeconds()));
#else
    return time;
#endif
}

class OpenSSLX509ExtensionI : public IceSSL::X509Extension
{

public:

    OpenSSLX509ExtensionI(struct X509_extension_st*, const string&, x509_st*);
    ~OpenSSLX509ExtensionI();
    virtual bool isCritical() const;
    virtual string getOID() const;
    virtual vector<Ice::Byte> getData() const;

private:

    struct X509_extension_st* _extension;
    string _oid;
    x509_st* _cert;
};

class OpenSSLCertificateI : public IceSSL::OpenSSL::Certificate,
                            public CertificateI,
                            public IceUtil::Mutex
{
public:

    OpenSSLCertificateI(x509_st*);
    ~OpenSSLCertificateI();

    virtual bool operator==(const IceSSL::Certificate&) const;

    virtual vector<Ice::Byte> getAuthorityKeyIdentifier() const;
    virtual vector<Ice::Byte> getSubjectKeyIdentifier() const;
    virtual bool verify(const IceSSL::CertificatePtr&) const;
    virtual string encode() const;

#  ifdef ICE_CPP11_MAPPING
    virtual chrono::system_clock::time_point getNotAfter() const;
    virtual chrono::system_clock::time_point getNotBefore() const;
#  else
    virtual IceUtil::Time getNotAfter() const;
    virtual IceUtil::Time getNotBefore() const;
#  endif
    virtual string getSerialNumber() const;
    virtual IceSSL::DistinguishedName getIssuerDN() const;
    virtual vector<pair<int, string> > getIssuerAlternativeNames() const;
    virtual IceSSL::DistinguishedName getSubjectDN() const;
    virtual vector<pair<int, string> > getSubjectAlternativeNames() const;
    virtual int getVersion() const;
    virtual x509_st* getCert() const;

protected:

    virtual void loadX509Extensions() const;

private:

    x509_st* _cert;
};

} // end anonymous namespace

OpenSSLX509ExtensionI::OpenSSLX509ExtensionI(struct X509_extension_st* extension, const string& oid, x509_st* cert):
    _extension(extension),
    _oid(oid),
    _cert(cert)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
    CRYPTO_add(&_cert->references, 1, CRYPTO_LOCK_X509);
#else
    X509_up_ref(_cert);
#endif
}

OpenSSLX509ExtensionI::~OpenSSLX509ExtensionI()
{
    X509_free(_cert);
}

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

vector<Ice::Byte>
OpenSSLX509ExtensionI::getData() const
{
    vector<Ice::Byte> data;
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
    if(!_cert)
    {
#ifdef ICE_CPP11_MAPPING
        throw invalid_argument("Invalid certificate reference");
#else
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Invalid certificate reference");
#endif
    }
}

OpenSSLCertificateI::~OpenSSLCertificateI()
{
    if(_cert)
    {
        X509_free(_cert);
    }
}

bool
OpenSSLCertificateI::operator==(const IceSSL::Certificate& r) const
{
    const OpenSSLCertificateI* p = dynamic_cast<const OpenSSLCertificateI*>(&r);
    if(!p)
    {
        return false;
    }

    return X509_cmp(_cert, p->_cert) == 0;
}

vector<Ice::Byte>
OpenSSLCertificateI::getAuthorityKeyIdentifier() const
{
    vector<Ice::Byte> keyid;
    int index = X509_get_ext_by_NID(_cert, NID_authority_key_identifier, -1);
    if(index >= 0)
    {
        X509_EXTENSION* ext = X509_get_ext(_cert, index);
        if(ext)
        {
            AUTHORITY_KEYID* decoded = (AUTHORITY_KEYID*)X509V3_EXT_d2i(ext);
            if(!decoded)
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

vector<Ice::Byte>
OpenSSLCertificateI::getSubjectKeyIdentifier() const
{
    vector<Ice::Byte> keyid;
    int index = X509_get_ext_by_NID(_cert, NID_subject_key_identifier, -1);
    if(index >= 0)
    {
        X509_EXTENSION* ext = X509_get_ext(_cert, index);
        if(ext)
        {
            ASN1_OCTET_STRING* decoded = static_cast<ASN1_OCTET_STRING*>(X509V3_EXT_d2i(ext));
            if(!decoded)
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
    if(c)
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
    if(i <= 0)
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

#  ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#  else
IceUtil::Time
#  endif
OpenSSLCertificateI::getNotAfter() const
{
    return ASMUtcTimeToTime(X509_get_notAfter(_cert));
}

#  ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#  else
IceUtil::Time
#  endif
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
    return IceSSL::DistinguishedName(IceSSL::RFC2253::parseStrict(convertX509NameToString(X509_get_issuer_name(_cert))));
}

vector<pair<int, string> >
OpenSSLCertificateI::getIssuerAlternativeNames() const
{
    return convertGeneralNames(reinterpret_cast<GENERAL_NAMES*>(X509_get_ext_d2i(_cert, NID_issuer_alt_name, 0, 0)));
}

IceSSL::DistinguishedName
OpenSSLCertificateI::getSubjectDN() const
{
    return IceSSL::DistinguishedName(IceSSL::RFC2253::parseStrict(convertX509NameToString(X509_get_subject_name(_cert))));
}

vector<pair<int, string> >
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
    IceUtil::Mutex::Lock sync(*this);
    if(_extensions.empty())
    {
        int sz = X509_get_ext_count(_cert);
        for(int i = 0; i < sz; i++)
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
            _extensions.push_back(ICE_DYNAMIC_CAST(IceSSL::X509Extension,
                ICE_MAKE_SHARED(OpenSSLX509ExtensionI, ext, oid, _cert)));
        }
    }
}

IceSSL::OpenSSL::CertificatePtr
IceSSL::OpenSSL::Certificate::create(x509_st* cert)
{
    return ICE_MAKE_SHARED(OpenSSLCertificateI, cert);
}

IceSSL::OpenSSL::CertificatePtr
IceSSL::OpenSSL::Certificate::load(const std::string& file)
{
    BIO* cert = BIO_new(BIO_s_file());
    if(BIO_read_filename(cert, file.c_str()) <= 0)
    {
        BIO_free(cert);
        throw CertificateReadException(__FILE__, __LINE__, "error opening file");
    }

    x509_st* x = PEM_read_bio_X509(cert, ICE_NULLPTR, ICE_NULLPTR, ICE_NULLPTR);
    if(x == ICE_NULLPTR)
    {
        BIO_free(cert);
        throw CertificateReadException(__FILE__, __LINE__, "error reading file:\n" + getSslErrors(false));
    }
    BIO_free(cert);
    return ICE_MAKE_SHARED(OpenSSLCertificateI, x);
}

IceSSL::OpenSSL::CertificatePtr
IceSSL::OpenSSL::Certificate::decode(const std::string& encoding)
{
    BIO *cert = BIO_new_mem_buf(static_cast<void*>(const_cast<char*>(&encoding[0])), static_cast<int>(encoding.size()));
    x509_st* x = PEM_read_bio_X509(cert, ICE_NULLPTR, ICE_NULLPTR, ICE_NULLPTR);
    if(x == ICE_NULLPTR)
    {
        BIO_free(cert);
        throw CertificateEncodingException(__FILE__, __LINE__, getSslErrors(false));
    }
    BIO_free(cert);
    return ICE_MAKE_SHARED(OpenSSLCertificateI, x);
}
