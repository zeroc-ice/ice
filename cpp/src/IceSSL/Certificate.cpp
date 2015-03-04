// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/StringUtil.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/Util.h>
#include <IceSSL/RFC2253.h>
#include <Ice/Object.h>

#include <openssl/x509v3.h>
#include <openssl/pem.h>

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


using namespace std;
using namespace Ice;
using namespace IceSSL;

const char* IceSSL::CertificateReadException::_name = "IceSSL::CertificateReadException";

CertificateReadException::CertificateReadException(const char* file, int line, const string& r) :
    Exception(file, line),
    reason(r)
{
}

CertificateReadException::~CertificateReadException() throw()
{
}

string
CertificateReadException::ice_name() const
{
    return _name;
}

CertificateReadException* 
CertificateReadException::ice_clone() const
{
    return new CertificateReadException(*this);
}

void
CertificateReadException::ice_throw() const
{
    throw *this;
}

const char* IceSSL::CertificateEncodingException::_name = "IceSSL::CertificateEncodingException";

CertificateEncodingException::CertificateEncodingException(const char* file, int line, const string& r) :
    Exception(file, line),
    reason(r)
{
}

CertificateEncodingException::~CertificateEncodingException() throw()
{
}

string
CertificateEncodingException::ice_name() const
{
    return _name;
}

CertificateEncodingException* 
CertificateEncodingException::ice_clone() const
{
    return new CertificateEncodingException(*this);
}

void
CertificateEncodingException::ice_throw() const
{
    throw *this;
}

namespace
{

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

}

static IceUtil::Time
ASMUtcTimeToIceUtilTime(const ASN1_UTCTIME* s)
{
    struct tm tm;
    int offset;
    
    memset(&tm, '\0', sizeof tm);
    
#define g2(p) (((p)[0]-'0')*10+(p)[1]-'0')
    tm.tm_year = g2(s->data);
    if(tm.tm_year < 50)
        tm.tm_year += 100;
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
#undef g2

    //
    // If timegm was on all systems this code could be
    // return IceUtil::Time::seconds(timegm(&tm) - offset*60);
    //
    // Windows doesn't support the re-entrant _r versions.
    //
    time_t tzone;
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(mut);
        time_t now = time(0);
        tzone = mktime(localtime(&now)) - mktime(gmtime(&now));
    }
    return IceUtil::Time::seconds(mktime(&tm) - offset*60 + tzone);
}

static string
convertX509NameToString(X509NAME* name)
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
    if(gens == 0)
    {
        return alt;
    }
    for(int i = 0; i < sk_GENERAL_NAME_num(gens); ++i)
    {
        GENERAL_NAME* gen = sk_GENERAL_NAME_value(gens, i);
        pair<int, string> p;
        p.first = gen->type;
        switch(gen->type)
        {
        case GEN_EMAIL:
        {
            ASN1_IA5STRING* str = gen->d.rfc822Name;
            if(str && str->type == V_ASN1_IA5STRING && str->data && str->length > 0)
            {
                p.second = string(reinterpret_cast<const char*>(str->data), str->length);
            }
            break;
        }
        case GEN_DNS:
        {
            ASN1_IA5STRING* str = gen->d.dNSName;
            if(str && str->type == V_ASN1_IA5STRING && str->data && str->length > 0)
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
            if(str && str->type == V_ASN1_IA5STRING && str->data && str->length > 0)
            {
                p.second = string(reinterpret_cast<const char*>(str->data), str->length);
            }
            break;
        }
        case GEN_IPADD:
        {
            ASN1_OCTET_STRING* addr = gen->d.iPAddress;
            // TODO: Support IPv6 someday.
            if(addr && addr->type == V_ASN1_OCTET_STRING && addr->data && addr->length == 4)
            {
                ostringstream ostr;
                for(int j = 0; j < 4; ++j)
                {
                    if(j > 0)
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

const char* ParseException::_name = "IceSSL::ParseException";

ParseException::ParseException(const char* file, int line, const string& r) :
    Exception(file, line),
    reason(r)
{
}

ParseException::~ParseException() throw()
{
}

string
ParseException::ice_name() const
{
    return _name;
}

ParseException* 
ParseException::ice_clone() const
{
    return new ParseException(*this);
}

void
ParseException::ice_throw() const
{
    throw *this;
}

DistinguishedName::DistinguishedName(X509NAME* name) :
    _rdns(RFC2253::parseStrict(convertX509NameToString(name)))
{
    unescape();
}

DistinguishedName::DistinguishedName(const string& dn) :
    _rdns(RFC2253::parseStrict(dn))
{
    unescape();
}

DistinguishedName::DistinguishedName(const list<pair<string, string> >& rdns) :
    _rdns(rdns)
{
    unescape();
}

bool
DistinguishedName::operator==(const DistinguishedName& other) const
{
    return other._unescaped == _unescaped;
}

bool
DistinguishedName::operator!=(const DistinguishedName& other) const
{
    return other._unescaped != _unescaped;
}

bool
DistinguishedName::operator<(const DistinguishedName& other) const
{
    return other._unescaped < _unescaped;
}

bool
DistinguishedName::match(const DistinguishedName& other) const
{
    for(list< pair<string, string> >::const_iterator p = other._unescaped.begin(); p != other._unescaped.end(); ++p)
    {
        bool found = false;
        for(list< pair<string, string> >::const_iterator q = _unescaped.begin(); q != _unescaped.end(); ++q)
        {
            if(p->first == q->first)
            {
                found = true;
                if(p->second != q->second)
                {
                    return false;
                }
            }
        }
        if(!found)
        {
            return false;
        }
    }
    return true;
}

//
// This always produces the same output as the input DN -- the type of
// escaping is not changed.
//
DistinguishedName::operator string() const
{
    ostringstream os;
    bool first = true;
    for(list< pair<string, string> >::const_iterator p = _rdns.begin(); p != _rdns.end(); ++p)
    {
        if(!first)
        {
            os << ",";
        }
        first = false;
        os << p->first << "=" << p->second;
    }
    return os.str();
}

void
DistinguishedName::unescape()
{
    for(list< pair<string, string> >::const_iterator q = _rdns.begin(); q != _rdns.end(); ++q)
    {
        pair<string, string> rdn = *q;
        rdn.second = RFC2253::unescape(rdn.second);
        _unescaped.push_back(rdn);
    }
}

PublicKey::PublicKey(EVP_PKEY* key) :
    _key(key)
{
}

PublicKey::~PublicKey()
{
    EVP_PKEY_free(_key);
}

EVP_PKEY*
PublicKey::key() const
{
    return _key;
}

//
// The caller is responsible for incrementing the reference count.
//
Certificate::Certificate(X509* cert) :
    _cert(cert)
{
    assert(_cert != 0);
}

Certificate::~Certificate()
{
    X509_free(_cert);
}

CertificatePtr
Certificate::load(const string& file)
{
    BIO *cert = BIO_new(BIO_s_file());
    if(BIO_read_filename(cert, file.c_str()) <= 0)
    {
        BIO_free(cert);
        throw CertificateReadException(__FILE__, __LINE__, "error opening file");
    }
    
    X509* x = PEM_read_bio_X509_AUX(cert, NULL, NULL, NULL);
    if(x == NULL)
    {
        BIO_free(cert);
        throw CertificateReadException(__FILE__, __LINE__, "error reading file:\n" + getSslErrors(false));
    }
    BIO_free(cert);
    return new Certificate(x);
}

CertificatePtr
Certificate::decode(const string& encoding)
{
    BIO *cert = BIO_new_mem_buf(static_cast<void*>(const_cast<char*>(&encoding[0])), static_cast<int>(encoding.size()));
    X509* x = PEM_read_bio_X509_AUX(cert, NULL, NULL, NULL);
    if(x == NULL)
    {
        BIO_free(cert);
        throw CertificateReadException(__FILE__, __LINE__, "error decoding certificate:\n" + getSslErrors(false));
    }
    BIO_free(cert);
    return new Certificate(x);
}

bool
Certificate::operator==(const Certificate& other) const
{
    return X509_cmp(_cert, other._cert) == 0;
}

bool
Certificate::operator!=(const Certificate& other) const
{
    return X509_cmp(_cert, other._cert) != 0;
}

PublicKeyPtr
Certificate::getPublicKey() const
{
    return new PublicKey(X509_get_pubkey(_cert));
}

bool
Certificate::verify(const PublicKeyPtr& key) const
{
    return X509_verify(_cert, key->key()) > 0;
}

string
Certificate::encode() const
{
    BIO* out = BIO_new(BIO_s_mem());
    int i = PEM_write_bio_X509_AUX(out, _cert);
    if(i <= 0)
    {
        BIO_free(out);
        throw CertificateEncodingException(__FILE__, __LINE__, getSslErrors(false));
    }
    BUF_MEM* p;
    BIO_get_mem_ptr(out, &p);
    string result = string(p->data, p->length);
    BIO_free(out);
    return result;
}

bool
Certificate::checkValidity() const
{
    IceUtil::Time now = IceUtil::Time::now();
    return now > getNotBefore() && now <= getNotAfter();
}

bool
Certificate::checkValidity(const IceUtil::Time& now) const
{
    return now > getNotBefore() && now <= getNotAfter();
}

IceUtil::Time
Certificate::getNotAfter() const
{
    return ASMUtcTimeToIceUtilTime(X509_get_notAfter(_cert));
}

IceUtil::Time
Certificate::getNotBefore() const
{
    return ASMUtcTimeToIceUtilTime(X509_get_notBefore(_cert));
}

string
Certificate::getSerialNumber() const
{
    BIGNUM* bn = ASN1_INTEGER_to_BN(X509_get_serialNumber(_cert), 0);
    char* dec = BN_bn2dec(bn);
    string result = dec;
    OPENSSL_free(dec);
    BN_free(bn);

    return result;
}

//string
//Certificate::getSigAlgName() const
//{
//}

//string
//Certificate::getSigAlgOID() const
//{
//}

DistinguishedName
Certificate::getIssuerDN() const
{
    return DistinguishedName(X509_get_issuer_name(_cert));
}

vector<pair<int, string> >
Certificate::getIssuerAlternativeNames()
{
    return convertGeneralNames(reinterpret_cast<GENERAL_NAMES*>(
        X509_get_ext_d2i(_cert, NID_issuer_alt_name, 0, 0)));
}

DistinguishedName
Certificate::getSubjectDN() const
{
    return DistinguishedName(X509_get_subject_name(_cert));
}

vector<pair<int, string> >
Certificate::getSubjectAlternativeNames()
{
    return convertGeneralNames(
        reinterpret_cast<GENERAL_NAMES*>(X509_get_ext_d2i(_cert, NID_subject_alt_name, 0, 0)));
}

int
Certificate::getVersion() const
{
    return static_cast<int>(X509_get_version(_cert));
}

string
Certificate::toString() const
{
    ostringstream os;
    os << "serial: " << getSerialNumber() << "\n";
    os << "issuer: " << string(getIssuerDN()) << "\n";
    os << "subject: " << string(getSubjectDN()) << "\n";
    os << "notBefore: " << getNotBefore().toDateTime() << "\n";
    os << "notAfter: " << getNotAfter().toDateTime();

    return os.str();
}

X509*
Certificate::getCert() const
{
    return _cert;
}
