// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
#include <Ice/Base64.h>
#include <Ice/StringConverter.h>
#include <IceUtil/Time.h>

#if defined(ICE_USE_OPENSSL)
#  include <openssl/x509v3.h>
#  include <openssl/pem.h>
//
// Avoid old style cast warnings from OpenSSL macros
//
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(ICE_USE_SECURE_TRANSPORT)
#  include <Security/Security.h>
#elif defined(ICE_OS_UWP)
#  include <ppltasks.h>
#  include <nserror.h>
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

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;

#ifdef ICE_OS_UWP
using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Security::Cryptography;
#endif

#if defined(ICE_USE_SECURE_TRANSPORT) || defined(ICE_USE_SCHANNEL)

//
// Map a certificate OID to its alias
//
struct CertificateOID
{
    const char* name;
    const char* alias;
};

const CertificateOID certificateOIDS[] =
{
    {"2.5.4.3", "CN"},
    {"2.5.4.4", "SN"},
    {"2.5.4.5", "DeviceSerialNumber"},
    {"2.5.4.6", "C"},
    {"2.5.4.7", "L"},
    {"2.5.4.8", "ST"},
    {"2.5.4.9", "STREET"},
    {"2.5.4.10", "O"},
    {"2.5.4.11", "OU"},
    {"2.5.4.12", "T"},
    {"2.5.4.42", "G"},
    {"2.5.4.43", "I"},
    {"1.2.840.113549.1.9.8", "unstructuredAddress"},
    {"1.2.840.113549.1.9.2", "unstructuredName"},
    {"1.2.840.113549.1.9.1", "emailAddress"},
    {"0.9.2342.19200300.100.1.25", "DC"}
};
const int certificateOIDSSize = sizeof(certificateOIDS) / sizeof(CertificateOID);

#endif

#if defined(ICE_USE_SECURE_TRANSPORT)

string
certificateOIDAlias(const string& name)
{
    for(int i = 0; i < certificateOIDSSize; ++i)
    {
        const CertificateOID* certificateOID = &certificateOIDS[i];
        assert(certificateOID);
        if(name == certificateOID->name)
        {
            return certificateOID->alias;
        }
    }
    return name;
}

string
escapeX509Name(const string& name)
{
    ostringstream os;
    for(string::const_iterator i = name.begin(); i != name.end(); ++i)
    {
        switch(*i)
        {
            case ',':
            case '=':
            case '+':
            case '<':
            case '>':
            case '#':
            case ';':
            {
                os << '\\';
            }
            default:
            {
                break;
            }
        }
        os << *i;
    }
    return os.str();
}

#if !defined(ICE_USE_SECURE_TRANSPORT_IOS)

//
// Map alternative name alias to its types.
//
const char* certificateAlternativeNameTypes[] = {"", "Email Address", "DNS Name", "", "Directory Name", "", "URI",
                                                 "IP Address"};
const int certificateAlternativeNameTypesSize = sizeof(certificateAlternativeNameTypes) / sizeof(char*);

int
certificateAlternativeNameType(const string& alias)
{
    if(!alias.empty())
    {
        for(int i = 0; i < certificateAlternativeNameTypesSize; ++i)
        {
            if(alias == certificateAlternativeNameTypes[i])
            {
                return i;
            }
        }
    }
    return -1; // Not supported
}

DistinguishedName
getX509Name(SecCertificateRef cert, CFTypeRef key)
{
    assert(key == kSecOIDX509V1IssuerName || key == kSecOIDX509V1SubjectName);
    list<pair<string, string> > rdnPairs;
    UniqueRef<CFDictionaryRef> property(getCertificateProperty(cert, key));
    if(property)
    {
        CFArrayRef dn = static_cast<CFArrayRef>(CFDictionaryGetValue(property.get(), kSecPropertyKeyValue));
        int size = CFArrayGetCount(dn);
        for(int i = 0; i < size; ++i)
        {
            CFDictionaryRef dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(dn, i));
            rdnPairs.push_front(make_pair(
                certificateOIDAlias(
                    fromCFString((static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyLabel))))),
                escapeX509Name(
                    fromCFString(static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyValue))))));
        }
    }
    return DistinguishedName(rdnPairs);
}

vector<pair<int, string> >
getX509AltName(SecCertificateRef cert, CFTypeRef key)
{
    assert(key == kSecOIDIssuerAltName || key == kSecOIDSubjectAltName);
    UniqueRef<CFDictionaryRef> property(getCertificateProperty(cert, key));

    vector<pair<int, string> > pairs;
    if(property)
    {
        CFArrayRef names = static_cast<CFArrayRef>(CFDictionaryGetValue(property.get(), kSecPropertyKeyValue));
        int size = CFArrayGetCount(names);

        for(int i = 0; i < size; ++i)
        {
            CFDictionaryRef dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(names, i));

            int type = certificateAlternativeNameType(
                fromCFString(static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyLabel))));
            if(type != -1)
            {
                CFStringRef v = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyValue));
                CFStringRef t = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kSecPropertyKeyType));
                if(CFEqual(t, kSecPropertyTypeString) || CFEqual(t, kSecPropertyTypeTitle))
                {
                    pairs.push_back(make_pair(type, fromCFString(v)));
                }
                else if(CFEqual(t, kSecPropertyTypeURL))
                {
                    pairs.push_back(make_pair(type, fromCFString(CFURLGetString((CFURLRef)v))));
                }
                else if(CFEqual(t, kSecPropertyTypeSection))
                {
                    CFArrayRef section = (CFArrayRef)v;
                    ostringstream os;
                    for(int i = 0, count = CFArrayGetCount(section); i < count;)
                    {
                        CFDictionaryRef d = (CFDictionaryRef)CFArrayGetValueAtIndex(section, i);

                        CFStringRef sectionLabel = static_cast<CFStringRef>(CFDictionaryGetValue(d, kSecPropertyKeyLabel));
                        CFStringRef sectionValue = static_cast<CFStringRef>(CFDictionaryGetValue(d, kSecPropertyKeyValue));

                        os << certificateOIDAlias(fromCFString(sectionLabel)) << "=" << fromCFString(sectionValue);
                        if(++i < count)
                        {
                            os << ",";
                        }
                    }
                    pairs.push_back(make_pair(type, os.str()));
                }
            }
        }
    }
    return pairs;
}

#ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#else
IceUtil::Time
#endif
getX509Date(SecCertificateRef cert, CFTypeRef key)
{
    assert(key == kSecOIDX509V1ValidityNotAfter || key == kSecOIDX509V1ValidityNotBefore);
    UniqueRef<CFDictionaryRef> property(getCertificateProperty(cert, key));
    CFAbsoluteTime seconds = 0;
    if(property)
    {
        CFNumberRef date = static_cast<CFNumberRef>(CFDictionaryGetValue(property.get(), kSecPropertyKeyValue));
        CFNumberGetValue(date, kCFNumberDoubleType, &seconds);
    }

    IceUtil::Time time = IceUtil::Time::secondsDouble(kCFAbsoluteTimeIntervalSince1970 + seconds);

#ifdef ICE_CPP11_MAPPING
    return chrono::system_clock::time_point(chrono::microseconds(time.toMicroSeconds()));
#else
    return time;
#endif
}

string
getX509String(SecCertificateRef cert, CFTypeRef key)
{
    assert(key == kSecOIDX509V1SerialNumber || key == kSecOIDX509V1Version);
    UniqueRef<CFDictionaryRef> property(getCertificateProperty(cert, key));
    return property ? 
        fromCFString(static_cast<CFStringRef>(CFDictionaryGetValue(property.get(), kSecPropertyKeyValue))) : "";
}

#else // IOS

//
// ASN1Parser to pase the subject/issuer ASN.1 DER encoded attributes on iOS.
//
class ASN1Parser
{
public:

    ASN1Parser(CFDataRef data) : _data(CFDataGetBytePtr(data)), _length(CFDataGetLength(data)), _p(_data), _next(0)
    {
    }

    list<pair<string, string> >
    parse()
    {
        list<pair<string, string> > rdns;
        while(_p < _data + _length)
        {
            switch(parseByte())
            {
                case 0x06: // OID
                {
                    _rdn.first = parseOID();
                    break;
                }
                case 0x12: // NumericString
                case 0x13: // PrintableString
                case 0x0C: // UTF8String
                case 0x16: // IA5String
                {
                    _rdn.second = escapeX509Name(parseUTF8String());
                    break;
                }
                case 0x30: // SEQUENCE
                case 0x31: // SET
                {
                    int length = parseLength(0);
                    _next = _p + length;
                    if(_next > _data + _length)
                    {
                        throw CertificateEncodingException(__FILE__, __LINE__, "invalid length");
                    }
                    break;
                }
                default:
                {
                    // Unsupported tag, skip the SET.
                    if(!_next)
                    {
                        return rdns;
                    }
                    _p = _next;
                    _next = 0;
                    break;
                }
            }
            if(_p == _next)
            {
                rdns.push_back(_rdn);
            }
        }
        return rdns;
    }

    string
    parseOID()
    {
        int length = parseLength(1);
        ostringstream oid;
        unsigned char c = parseByte();
        oid << c / 40 << "." << c % 40;
        while(--length > 0)
        {
            if((*_p & 0x80) == 0)
            {
                oid << "." << static_cast<int>(parseByte());
            }
            else
            {
                uint64_t result = (uint64_t)(*_p & 127);
                while(parseByte() & 128)
                {
                    result = (result << 7) | (uint64_t)(*_p & 127);
                    --length;
                }
                oid << "." << result;
            }
        }
        return certificateOIDAlias(oid.str());
    }

    string
    parseUTF8String()
    {
        int length = parseLength(0);
        string v(reinterpret_cast<const char*>(_p), length);
        _p += length;
        return v;
    }

    int
    parseLength(int required)
    {
        int length = 0;
        if((*_p & 0x80) == 0)
        {
            length = static_cast<int>(parseByte());
        }
        else
        {
            int nbytes = static_cast<int>(parseByte());
            for(int i = 0; i < nbytes; ++i)
            {
                length = length * 256 + parseByte();
            }
        }
        if((required > 0 && length < required) || (_p + length > _data + _length))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, "invalid length");
        }
        return length;
    }

    unsigned char
    parseByte()
    {
        if(_p >= _data + _length)
        {
            throw CertificateEncodingException(__FILE__, __LINE__, "invalid length");
        }
        unsigned char b = *_p++;
        return b;
    }

private:

    const unsigned char* _data;
    const size_t _length;
    const unsigned char* _p;
    const unsigned char* _next;
    pair<string, string> _rdn;
    list<pair<string, string> > _rdns;
};
#endif

#elif defined(ICE_USE_SCHANNEL) || defined(ICE_OS_UWP)

const Ice::Long TICKS_PER_MSECOND = 10000LL;
const Ice::Long MSECS_TO_EPOCH = 11644473600000LL;

#if defined(ICE_USE_SCHANNEL)
void
loadCertificate(PCERT_SIGNED_CONTENT_INFO* cert, const char* buffer, DWORD length)
{
    DWORD outLength = length;
    vector<BYTE> outBuffer;
    outBuffer.resize(outLength);

    if(!CryptStringToBinary(buffer, length, CRYPT_STRING_BASE64HEADER, &outBuffer[0], &outLength, 0, 0))
    {
        //
        // Base64 data should always be bigger than binary
        //
        assert(GetLastError() != ERROR_MORE_DATA);
        throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    DWORD decodedLeng = 0;
    if(!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_CERT, &outBuffer[0], outLength, CRYPT_DECODE_ALLOC_FLAG, 0,
                            cert, &decodedLeng))
    {
        throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }
}

void
loadCertificate(PCERT_SIGNED_CONTENT_INFO* cert, const string& file)
{
    vector<char> buffer;
    readFile(file, buffer);
    if(buffer.empty())
    {
        throw CertificateReadException(__FILE__, __LINE__, "certificate file " + file + " is empty");
    }
    loadCertificate(cert, &buffer[0], static_cast<DWORD>(buffer.size()));
}

#ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#else
IceUtil::Time
#endif
filetimeToTime(FILETIME ftime)
{
    Ice::Long value = 0;
    DWORD* dest = reinterpret_cast<DWORD*>(&value);
    *dest++ = ftime.dwLowDateTime;
    *dest = ftime.dwHighDateTime;

    IceUtil::Time time = IceUtil::Time::milliSeconds((value / TICKS_PER_MSECOND) - MSECS_TO_EPOCH);

#ifdef ICE_CPP11_MAPPING
    return chrono::system_clock::time_point(chrono::microseconds(time.toMicroSeconds()));
#else
    return time;
#endif

}

string
certNameToString(CERT_NAME_BLOB* name)
{
    assert(name);
    DWORD length = 0;
    if(!(length = CertNameToStr(X509_ASN_ENCODING, name, CERT_OID_NAME_STR|CERT_NAME_STR_REVERSE_FLAG, 0, 0)))
    {
        throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    vector<char> buffer(length);
    if(!CertNameToStr(X509_ASN_ENCODING, name, CERT_OID_NAME_STR|CERT_NAME_STR_REVERSE_FLAG, &buffer[0], length))
    {
        throw CertificateEncodingException(__FILE__, __LINE__,  IceUtilInternal::lastErrorToString());
    }

    string s(&buffer[0]);
    for(int i = 0; i < certificateOIDSSize; ++i)
    {
        const CertificateOID* certificateOID = &certificateOIDS[i];
        assert(certificateOID);
        const string name = string(certificateOID->name) + "=";
        const string alias = string(certificateOID->alias) + "=";
        size_t pos = 0;
        while((pos = s.find(name, pos)) != string::npos)
        {
            s.replace(pos, name.size(), alias);
        }
    }
    return s;
}

vector<pair<int, string> >
certificateAltNames(CERT_INFO* certInfo, LPCSTR altNameOID)
{
    vector<pair<int, string> > altNames;

    PCERT_EXTENSION extension = CertFindExtension(altNameOID, certInfo->cExtension, certInfo->rgExtension);
    if(extension)
    {
        CERT_ALT_NAME_INFO* altName;
        DWORD length = 0;
        if(!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME, extension->Value.pbData,
                                extension->Value.cbData, CRYPT_DECODE_ALLOC_FLAG, 0, &altName, &length))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
        }

        for(DWORD i = 0; i < altName->cAltEntry; ++i)
        {
            CERT_ALT_NAME_ENTRY* entry = &altName->rgAltEntry[i];

            switch(entry->dwAltNameChoice)
            {
                case CERT_ALT_NAME_RFC822_NAME:
                {
                    altNames.push_back(make_pair(AltNameEmail, wstringToString(entry->pwszRfc822Name)));
                    break;
                }
                case CERT_ALT_NAME_DNS_NAME:
                {
                    altNames.push_back(make_pair(AltNameDNS, wstringToString(entry->pwszDNSName)));
                    break;
                }
                case CERT_ALT_NAME_DIRECTORY_NAME:
                {
                    altNames.push_back(make_pair(AltNameDirectory, certNameToString(&entry->DirectoryName)));
                    break;
                }
                case CERT_ALT_NAME_URL:
                {
                    altNames.push_back(make_pair(AltNameURL, wstringToString(entry->pwszURL)));
                    break;
                }
                case CERT_ALT_NAME_IP_ADDRESS:
                {
                    if(entry->IPAddress.cbData == 4)
                    {
                        //
                        // IPv4 address
                        //
                        ostringstream os;
                        Byte* src = reinterpret_cast<Byte*>(entry->IPAddress.pbData);
                        for(int j = 0; j < 4;)
                        {
                            int value = 0;
                            Byte* dest = reinterpret_cast<Byte*>(&value);
                            *dest = *src++;
                            os << value;
                            if(++j < 4)
                            {
                                os << ".";
                            }
                        }
                        altNames.push_back(make_pair(AltNAmeIP, os.str()));
                    }
                    //
                    // TODO IPv6 Address support.
                    //
                    break;
                }
                default:
                {
                    // Not supported
                    break;
                }
            }
        }
        LocalFree(altName);
    }
    return altNames;
}
#else

vector<pair<int, string> >
certificateAltNames(Windows::Security::Cryptography::Certificates::SubjectAlternativeNameInfo^ subAltNames)
{
    vector<pair<int, string> > altNames;
    if(subAltNames)
    {
        for(auto iter = subAltNames->EmailName->First(); iter->HasCurrent; iter->MoveNext())
        {
            altNames.push_back(make_pair(AltNameEmail, wstringToString(iter->Current->Data())));
        }
        for(auto iter = subAltNames->DnsName->First(); iter->HasCurrent; iter->MoveNext())
        {
            altNames.push_back(make_pair(AltNameDNS, wstringToString(iter->Current->Data())));
        }
        for(auto iter = subAltNames->Url->First(); iter->HasCurrent; iter->MoveNext())
        {
            altNames.push_back(make_pair(AltNameURL, wstringToString(iter->Current->Data())));
        }
        for(auto iter = subAltNames->IPAddress->First(); iter->HasCurrent; iter->MoveNext())
        {
            altNames.push_back(make_pair(AltNAmeIP, wstringToString(iter->Current->Data())));
        }
    }
    return altNames;
}
#endif

#endif

CertificateReadException::CertificateReadException(const char* file, int line, const string& r) :
    ExceptionHelper<CertificateReadException>(file, line),
    reason(r)
{
}

#ifndef ICE_CPP11_COMPILER
CertificateReadException::~CertificateReadException() throw()
{
}
#endif

string
CertificateReadException::ice_id() const
{
    return "::IceSSL::CertificateReadException";
}

#ifndef ICE_CPP11_MAPPING
CertificateReadException*
CertificateReadException::ice_clone() const
{
    return new CertificateReadException(*this);
}
#endif

#ifdef ICE_USE_SECURE_TRANSPORT
CertificateEncodingException::CertificateEncodingException(const char* file, int line, CFErrorRef err) :
    ExceptionHelper<CertificateEncodingException>(file, line)
{
    assert(err);
    reason = "certificate error:\n" + errorToString(err);
}
#endif

CertificateEncodingException::CertificateEncodingException(const char* file, int line, const string& r) :
    ExceptionHelper<CertificateEncodingException>(file, line),
    reason(r)
{
}

#ifndef ICE_CPP11_COMPILER
CertificateEncodingException::~CertificateEncodingException() throw()
{
}
#endif

string
CertificateEncodingException::ice_id() const
{
    return "::IceSSL::CertificateEncodingException";
}

#ifndef ICE_CPP11_MAPPING
CertificateEncodingException*
CertificateEncodingException::ice_clone() const
{
    return new CertificateEncodingException(*this);
}
#endif

#ifdef ICE_USE_OPENSSL

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
#  undef g2

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

    IceUtil::Time time = IceUtil::Time::seconds(mktime(&tm) - offset * 60 + tzone);

#ifdef ICE_CPP11_MAPPING
    return chrono::system_clock::time_point(chrono::microseconds(time.toMicroSeconds()));
#else
    return time;
#endif
}

static string
convertX509NameToString(X509_NAME* name)
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
#endif

ParseException::ParseException(const char* file, int line, const string& r) :
    ExceptionHelper<ParseException>(file, line),
    reason(r)
{
}

#ifndef ICE_CPP11_COMPILER
ParseException::~ParseException() throw()
{
}
#endif

string
ParseException::ice_id() const
{
    return "::IceSSL::ParseException";
}

#ifndef ICE_CPP11_MAPPING
ParseException*
ParseException::ice_clone() const
{
    return new ParseException(*this);
}
#endif

#ifdef ICE_USE_OPENSSL
DistinguishedName::DistinguishedName(X509NAME* name) : _rdns(RFC2253::parseStrict(convertX509NameToString(name)))
{
    unescape();
}
#endif

#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
DistinguishedName::DistinguishedName(CFDataRef data) : _rdns(ASN1Parser(data).parse())
{
    unescape();
}
#endif

DistinguishedName::DistinguishedName(const string& dn) : _rdns(RFC2253::parseStrict(dn))
{
    unescape();
}

DistinguishedName::DistinguishedName(const list<pair<string, string> >& rdns) : _rdns(rdns)
{
    unescape();
}

namespace IceSSL
{

bool
operator==(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return lhs._unescaped == rhs._unescaped;
}

bool
operator<(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return lhs._unescaped == rhs._unescaped;
}

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

bool
DistinguishedName::match(const string& other) const
{
    return match(DistinguishedName(other));
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

PublicKey::PublicKey(const CertificatePtr& cert, KeyRef key) :
    _cert(cert),
    _key(key)
{
    if(!_key)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Invalid key reference");
    }
}

//
// With SecureTransport the key is UniqueRef and will be automatically released.
// With SChannel the key is owned by the certificate and there is no need
// for release it.
//
#ifdef ICE_USE_OPENSSL
PublicKey::~PublicKey()
{
    if(_key)
    {
        EVP_PKEY_free(_key);
    }
}
#endif

KeyRef
PublicKey::key() const
{
#ifdef __APPLE__
    return _key.get();
#else
    return _key;
#endif
}

//
// The caller is responsible for incrementing the reference count.
//
Certificate::Certificate(X509CertificateRef cert) : _cert(cert)
{
    if(!_cert)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Invalid certificate reference");
    }

#if defined(ICE_USE_SCHANNEL)
    _certInfo = 0;
    try
    {
        //
        // Decode certificate info
        //
        DWORD length = 0;
        if(!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_CERT_TO_BE_SIGNED, _cert->ToBeSigned.pbData,
                                _cert->ToBeSigned.cbData, CRYPT_DECODE_ALLOC_FLAG, 0, &_certInfo, &length))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
        }
    }
    catch(...)
    {
        LocalFree(_cert);
        _cert = 0;
        throw;
    }
#endif
}

Certificate::~Certificate()
{
    if(_cert)
    {
#if defined(ICE_USE_SCHANNEL)
        LocalFree(_cert);
        if(_certInfo)
        {
            LocalFree(_certInfo);
        }
#elif defined(ICE_USE_OPENSSL)
        X509_free(_cert);
#endif
    }
}

CertificatePtr
Certificate::load(const string& file)
{
#if defined(ICE_USE_SECURE_TRANSPORT)
    string resolved;
    if(checkPath(file, "", false, resolved))
    {
        return ICE_MAKE_SHARED(Certificate, loadCertificate(resolved));
    }
    else
    {
        throw CertificateReadException(__FILE__, __LINE__, "error opening file " + file);
    }
#elif defined(ICE_USE_SCHANNEL)
    CERT_SIGNED_CONTENT_INFO* cert;
    loadCertificate(&cert, file);
    return ICE_MAKE_SHARED(Certificate, cert);
#elif defined(ICE_USE_OPENSSL)
    BIO *cert = BIO_new(BIO_s_file());
    if(BIO_read_filename(cert, file.c_str()) <= 0)
    {
        BIO_free(cert);
        throw CertificateReadException(__FILE__, __LINE__, "error opening file");
    }

    X509CertificateRef x = PEM_read_bio_X509_AUX(cert, ICE_NULLPTR, ICE_NULLPTR, ICE_NULLPTR);
    if(x == ICE_NULLPTR)
    {
        BIO_free(cert);
        throw CertificateReadException(__FILE__, __LINE__, "error reading file:\n" + getSslErrors(false));
    }
    BIO_free(cert);
    return ICE_MAKE_SHARED(Certificate, x);
#elif defined(ICE_OS_UWP)
    try
    {
        auto uri = ref new Uri(ref new String(stringToWstring(file).c_str()));
        auto file = create_task(StorageFile::GetFileFromApplicationUriAsync(uri)).get();
        auto buffer = create_task(FileIO::ReadTextAsync(file)).get();
        return Certificate::decode(wstringToString(buffer->Data()));
    }
    catch(Platform::Exception^ ex)
    {
        if(HRESULT_CODE(ex->HResult) == ERROR_FILE_NOT_FOUND)
        {
            throw CertificateReadException(__FILE__, __LINE__, "error opening file :" + file);
        }
        else
        {
            throw Ice::SyscallException(__FILE__, __LINE__, ex->HResult);
        }
    }
#else
#   error "Unknown platform"
#endif
}

CertificatePtr
Certificate::decode(const string& encoding)
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    string::size_type size, startpos, endpos = 0;
    startpos = encoding.find("-----BEGIN CERTIFICATE-----", endpos);
    if(startpos != string::npos)
    {
        startpos += sizeof("-----BEGIN CERTIFICATE-----");
        endpos = encoding.find("-----END CERTIFICATE-----", startpos);
        size = endpos - startpos;
    }
    else
    {
        startpos = 0;
        endpos = string::npos;
        size = encoding.size();
    }

    vector<unsigned char> data(IceInternal::Base64::decode(string(&encoding[startpos], size)));
    UniqueRef<CFDataRef> certdata(CFDataCreate(kCFAllocatorDefault, &data[0], data.size()));
    SecCertificateRef cert = SecCertificateCreateWithData(0, certdata.get());
    if(!cert)
    {
        assert(false);
        throw CertificateEncodingException(__FILE__, __LINE__, "certificate is not a valid PEM-encoded certificate");
    }
    return ICE_MAKE_SHARED(Certificate, cert);
#elif defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    UniqueRef<CFDataRef> data(
        CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
                                    reinterpret_cast<const UInt8*>(encoding.c_str()),
                                    encoding.size(), kCFAllocatorNull));

    SecExternalFormat format = kSecFormatUnknown;
    SecExternalItemType type = kSecItemTypeCertificate;

    SecItemImportExportKeyParameters params;
    memset(&params, 0, sizeof(params));
    params.version =  SEC_KEY_IMPORT_EXPORT_PARAMS_VERSION;

    UniqueRef<CFArrayRef> items;
    OSStatus err = SecItemImport(data.get(), 0, &format, &type, 0, &params, 0, &items.get());
    if(err)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, errorToString(err));
    }

    UniqueRef<SecKeychainItemRef> item;
    item.retain(static_cast<SecKeychainItemRef>(const_cast<void*>(CFArrayGetValueAtIndex(items.get(), 0))));
    assert(SecCertificateGetTypeID() == CFGetTypeID(item.get()));
    return ICE_MAKE_SHARED(Certificate, reinterpret_cast<SecCertificateRef>(item.release()));
#elif defined(ICE_USE_SCHANNEL)
    CERT_SIGNED_CONTENT_INFO* cert;
    loadCertificate(&cert, encoding.c_str(), static_cast<DWORD>(encoding.size()));
    return ICE_MAKE_SHARED(Certificate, cert);
#elif defined(ICE_USE_OPENSSL)
    BIO *cert = BIO_new_mem_buf(static_cast<void*>(const_cast<char*>(&encoding[0])), static_cast<int>(encoding.size()));
    X509CertificateRef x = PEM_read_bio_X509_AUX(cert, ICE_NULLPTR, ICE_NULLPTR, ICE_NULLPTR);
    if(x == ICE_NULLPTR)
    {
        BIO_free(cert);
        throw CertificateEncodingException(__FILE__, __LINE__, getSslErrors(false));
    }
    BIO_free(cert);
    return ICE_MAKE_SHARED(Certificate, x);
#elif defined(ICE_OS_UWP)
    string::size_type size, startpos, endpos = 0;
    startpos = encoding.find("-----BEGIN CERTIFICATE-----", endpos);
    if (startpos != string::npos)
    {
        startpos += sizeof("-----BEGIN CERTIFICATE-----");
        endpos = encoding.find("-----END CERTIFICATE-----", startpos);
        size = endpos - startpos;
    }
    else
    {
        startpos = 0;
        endpos = string::npos;
        size = encoding.size();
    }

    vector<unsigned char> data(IceInternal::Base64::decode(string(&encoding[startpos], size)));
    auto writer = ref new DataWriter();
    writer->WriteBytes(Platform::ArrayReference<unsigned char>(&data[0], static_cast<unsigned int>(data.size())));
    return make_shared<Certificate>(ref new Certificates::Certificate(writer->DetachBuffer()));
#else
#   error "Unknown platform"
#endif
}

bool
Certificate::operator==(const Certificate& other) const
{
#if defined(ICE_USE_SECURE_TRANSPORT)
    return CFEqual(_cert.get(), other._cert.get());
#elif defined(ICE_USE_SCHANNEL)
    return CertCompareCertificate(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, _certInfo, other._certInfo);
#elif defined(ICE_USE_OPENSSL)
    return X509_cmp(_cert, other._cert) == 0;
#elif defined(ICE_OS_UWP)
    return CryptographicBuffer::Compare(_cert->GetCertificateBlob(), other._cert->GetCertificateBlob());
#else
#   error "Unknown platform"
#endif
}

bool
Certificate::operator!=(const Certificate& other) const
{
    return !operator==(other);
}

PublicKeyPtr
Certificate::getPublicKey() const
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    return ICE_NULLPTR; // Not supported
#elif defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    UniqueRef<SecKeyRef> key;
    OSStatus err = SecCertificateCopyPublicKey(_cert.get(), &key.get());
    if(err)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, errorToString(err));
    }
    return ICE_MAKE_SHARED(PublicKey, ICE_SHARED_FROM_CONST_THIS(Certificate), key.release());
#elif defined(ICE_USE_SCHANNEL)
    return ICE_MAKE_SHARED(PublicKey, ICE_SHARED_FROM_CONST_THIS(Certificate), &_certInfo->SubjectPublicKeyInfo);
#elif defined(ICE_USE_OPENSSL)
    return ICE_MAKE_SHARED(PublicKey, ICE_SHARED_FROM_CONST_THIS(Certificate), X509_get_pubkey(_cert));
#elif defined(ICE_OS_UWP)
    return ICE_NULLPTR; // Not supported
#else
#   error "Unknown platform"
#endif
}

bool
Certificate::verify(const CertificatePtr& cert) const
{
#if defined(ICE_USE_SECURE_TRANSPORT)
    //
    // We first check if the given certificate subject match our certificate
    // issuer. Otherwhise when checking a certificate against itself
    // SecTrustEvaluate always returns it is valid.
    //
    bool valid = false;

#  if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    initializeAttributes();
    cert->initializeAttributes();
    valid = CFEqual(_issuer.get(), cert->_subject.get());
#  else
    UniqueRef<CFErrorRef> error;
    UniqueRef<CFDataRef> issuer(SecCertificateCopyNormalizedIssuerContent(_cert.get(), &error.get()));
    if(error)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, error.get());
    }
    UniqueRef<CFDataRef> subject(SecCertificateCopyNormalizedSubjectContent(cert->getCert(), &error.get()));
    if(error)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, error.get());
    }

    //
    // The certificate issuer must match the CA subject.
    //
    valid = CFEqual(issuer.get(), subject.get());
#  endif
    if(valid)
    {
        UniqueRef<SecPolicyRef> policy(SecPolicyCreateBasicX509());
        UniqueRef<SecTrustRef> trust;
        OSStatus err = 0;;
        if((err = SecTrustCreateWithCertificates(_cert.get(), policy.get(), &trust.get())))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, errorToString(err));
        }

        SecCertificateRef certs[1] = { cert->getCert() };
        UniqueRef<CFArrayRef> anchorCertificates(
            CFArrayCreate(kCFAllocatorDefault, (const void**)&certs, 1, &kCFTypeArrayCallBacks));
        if((err = SecTrustSetAnchorCertificates(trust.get(), anchorCertificates.get())))
        {
            throw CertificateEncodingException(__FILE__, __LINE__,  errorToString(err));
        }

        SecTrustResultType trustResult = kSecTrustResultInvalid;
        if((err = SecTrustEvaluate(trust.get(), &trustResult)))
        {
            throw CertificateEncodingException(__FILE__, __LINE__,  errorToString(err));
        }

        valid = trustResult == kSecTrustResultUnspecified;
    }
    return valid;
#elif defined(ICE_USE_SCHANNEL)
    BYTE* buffer = 0;
    DWORD length = 0;
    if(!CryptEncodeObjectEx(X509_ASN_ENCODING, X509_CERT, _cert, CRYPT_ENCODE_ALLOC_FLAG , 0, &buffer, &length))
    {
        throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    bool result = CryptVerifyCertificateSignature(0, X509_ASN_ENCODING, buffer, length, cert->getPublicKey()->key());
    LocalFree(buffer);
    return result;
#elif defined(ICE_USE_OPENSSL)
    return X509_verify(_cert, cert->getPublicKey()->key()) > 0;
#elif defined(ICE_OS_UWP)
    return false;
#else
#   error "Unknown platform"
#endif
}

#ifdef ICE_USE_OPENSSL
bool
Certificate::verify(const PublicKeyPtr& key) const
{
    return X509_verify(_cert, key->key()) > 0;
}
#endif

string
Certificate::encode() const
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    UniqueRef<CFDataRef> cert(SecCertificateCopyData(_cert.get()));
    vector<unsigned char> data(CFDataGetBytePtr(cert.get()), CFDataGetBytePtr(cert.get()) + CFDataGetLength(cert.get()));
    ostringstream os;
    os << "-----BEGIN CERTIFICATE-----\n";
    os << IceInternal::Base64::encode(data);
    os << "-----END CERTIFICATE-----\n";
    return os.str();
#elif defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    UniqueRef<CFDataRef> exported;
    OSStatus err = SecItemExport(_cert.get(), kSecFormatPEMSequence, kSecItemPemArmour, 0, &exported.get());
    if(err != noErr)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, errorToString(err));
    }
    return string(reinterpret_cast<const char*>(CFDataGetBytePtr(exported.get())), CFDataGetLength(exported.get()));
#elif defined(ICE_USE_SCHANNEL)
    string s;
    DWORD length = 0;
    BYTE* buffer = 0;
    try
    {
        if(!CryptEncodeObjectEx(X509_ASN_ENCODING, X509_CERT, _cert, CRYPT_ENCODE_ALLOC_FLAG , 0, &buffer, &length))
        {
            throw CertificateEncodingException(__FILE__, __LINE__,  IceUtilInternal::lastErrorToString());
        }

        DWORD encodedLength = 0;
        if(!CryptBinaryToString(buffer, length, CRYPT_STRING_BASE64HEADER | CRYPT_STRING_NOCR, 0, &encodedLength))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
        }

        std::vector<char> encoded;
        encoded.resize(encodedLength);
        if(!CryptBinaryToString(buffer, length, CRYPT_STRING_BASE64HEADER | CRYPT_STRING_NOCR, &encoded[0],
                                &encodedLength))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
        }
        LocalFree(buffer);
        buffer = 0;
        s.assign(&encoded[0]);
    }
    catch(...)
    {
        if(buffer)
        {
            LocalFree(buffer);
        }
        throw;
    }
    return s;
#elif defined(ICE_USE_OPENSSL)
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
#elif defined(ICE_OS_UWP)
    auto reader = Windows::Storage::Streams::DataReader::FromBuffer(_cert->GetCertificateBlob());
    std::vector<unsigned char> data(reader->UnconsumedBufferLength);
    if(!data.empty())
    {
        reader->ReadBytes(Platform::ArrayReference<unsigned char>(&data[0], static_cast<unsigned int>(data.size())));
    }
    ostringstream os;
    os << "-----BEGIN CERTIFICATE-----\n";
    os << IceInternal::Base64::encode(data);
    os << "-----END CERTIFICATE-----\n";
    return os.str();
#else
#   error "Unknown platform"
#endif
}

#if !defined(ICE_USE_SECURE_TRANSPORT_IOS)

bool
Certificate::checkValidity() const
{
#ifdef ICE_CPP11_MAPPING
    auto now = chrono::system_clock::now();
#else
    IceUtil::Time now = IceUtil::Time::now();
#endif
    return now > getNotBefore() && now <= getNotAfter();
}

bool
#ifdef ICE_CPP11_MAPPING
Certificate::checkValidity(const chrono::system_clock::time_point& now) const
#else
Certificate::checkValidity(const IceUtil::Time& now) const
#endif
{
    return now > getNotBefore() && now <= getNotAfter();
}

#ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#else
IceUtil::Time
#endif
Certificate::getNotAfter() const
{
#if defined(ICE_USE_SECURE_TRANSPORT)
    return getX509Date(_cert.get(), kSecOIDX509V1ValidityNotAfter);
#elif defined(ICE_USE_SCHANNEL)
    return filetimeToTime(_certInfo->NotAfter);
#elif defined(ICE_USE_OPENSSL)
    return ASMUtcTimeToTime(X509_get_notAfter(_cert));
#elif defined(ICE_OS_UWP)
    // Convert 100ns time from January 1, 1601 to ms from January 1, 1970
    IceUtil::Time time = IceUtil::Time::milliSeconds(_cert->ValidTo.UniversalTime / TICKS_PER_MSECOND - MSECS_TO_EPOCH);
#   ifdef ICE_CPP11_MAPPING
    return chrono::system_clock::time_point(chrono::microseconds(time.toMicroSeconds()));
#   else
    return time;
#   endif

#else
#   error "Unknown platform"
#endif
}

#ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#else
IceUtil::Time
#endif
Certificate::getNotBefore() const
{
#if defined(ICE_USE_SECURE_TRANSPORT)
    return getX509Date(_cert.get(), kSecOIDX509V1ValidityNotBefore);
#elif defined(ICE_USE_SCHANNEL)
    return filetimeToTime(_certInfo->NotBefore);
#elif defined(ICE_USE_OPENSSL)
    return ASMUtcTimeToTime(X509_get_notBefore(_cert));
#elif defined(ICE_OS_UWP)
    // Convert 100ns time from January 1, 1601 to ms from January 1, 1970
    IceUtil::Time time = IceUtil::Time::milliSeconds(_cert->ValidFrom.UniversalTime / TICKS_PER_MSECOND - MSECS_TO_EPOCH);
#   ifdef ICE_CPP11_MAPPING
    return chrono::system_clock::time_point(chrono::microseconds(time.toMicroSeconds()));
#   else
    return time;
#   endif

#else
#   error "Unknown platform"
#endif
}

#endif

string
Certificate::getSerialNumber() const
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    initializeAttributes();
    return _serial;
#elif defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    return getX509String(_cert.get(), kSecOIDX509V1SerialNumber);
#elif defined(ICE_USE_SCHANNEL)
    ostringstream os;
    for(int i = _certInfo->SerialNumber.cbData - 1; i >= 0; --i)
    {
        unsigned char c = _certInfo->SerialNumber.pbData[i];
        os.fill('0');
        os.width(2);
        os << hex << static_cast<int>(c);
        if(i)
        {
            os << ' ';
        }
    }
    return IceUtilInternal::toUpper(os.str());
#elif defined(ICE_USE_OPENSSL)
    BIGNUM* bn = ASN1_INTEGER_to_BN(X509_get_serialNumber(_cert), 0);
    char* dec = BN_bn2dec(bn);
    string result = dec;
    OPENSSL_free(dec);
    BN_free(bn);

    return result;
#elif defined(ICE_OS_UWP)
    ostringstream os;
    os.fill(0);
    os.width(2);
    for (unsigned int i = 0; i < _cert->SerialNumber->Length; i++)
    {
        os << hex << static_cast<int>(_cert->SerialNumber[i]);
    }
    return IceUtilInternal::toUpper(os.str());
#else
#   error "Unknown platform"
#endif
}

DistinguishedName
Certificate::getIssuerDN() const
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    initializeAttributes();
    return _issuer ? DistinguishedName(_issuer.get()) : DistinguishedName("");
#elif defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    return getX509Name(_cert.get(), kSecOIDX509V1IssuerName);
#elif defined(ICE_USE_SCHANNEL)
    return DistinguishedName(certNameToString(&_certInfo->Issuer));
#elif defined(ICE_USE_OPENSSL)
    return DistinguishedName(RFC2253::parseStrict(convertX509NameToString(X509_get_issuer_name(_cert))));
#elif defined(ICE_OS_UWP)
    ostringstream os;
    os << "CN=" << wstringToString(_cert->Issuer->Data());
    return DistinguishedName(os.str());
#else
#   error "Unknown platform"
#endif
}

#if !defined(ICE_USE_SECURE_TRANSPORT_IOS)
vector<pair<int, string> >
Certificate::getIssuerAlternativeNames()
{
#if defined(ICE_USE_SECURE_TRANSPORT)
    return getX509AltName(_cert.get(), kSecOIDIssuerAltName);
#elif defined(ICE_USE_SCHANNEL)
    return certificateAltNames(_certInfo, szOID_ISSUER_ALT_NAME2);
#elif defined(ICE_USE_OPENSSL)
    return convertGeneralNames(reinterpret_cast<GENERAL_NAMES*>(X509_get_ext_d2i(_cert, NID_issuer_alt_name, 0, 0)));
#elif defined(ICE_OS_UWP)
    return vector<pair<int, string> >(); // Not supported
#else
#   error "Unknown platform"
#endif
}
#endif

DistinguishedName
Certificate::getSubjectDN() const
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    initializeAttributes();
    if(_subject)
    {
        return DistinguishedName(_subject.get());
    }
    else
    {
        UniqueRef<CFStringRef> subjectSummary(SecCertificateCopySubjectSummary(_cert.get()));
        return DistinguishedName("CN=" + fromCFString(subjectSummary.get()));
    }
#elif defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    return getX509Name(_cert.get(), kSecOIDX509V1SubjectName);
#elif defined(ICE_USE_SCHANNEL)
    return DistinguishedName(certNameToString(&_certInfo->Subject));
#elif defined(ICE_USE_OPENSSL)
    return DistinguishedName(RFC2253::parseStrict(convertX509NameToString(X509_get_subject_name(_cert))));
#elif defined(ICE_OS_UWP)
    ostringstream os;
    os << "CN=" << wstringToString(_cert->Subject->Data());
    return DistinguishedName(os.str());
#else
#   error "Unknown platform"
#endif
}

#if !defined(ICE_USE_SECURE_TRANSPORT_IOS)
vector<pair<int, string> >
Certificate::getSubjectAlternativeNames()
{
#if defined(ICE_USE_SECURE_TRANSPORT)
    return getX509AltName(_cert.get(), kSecOIDSubjectAltName);
#elif defined(ICE_USE_SCHANNEL)
    return certificateAltNames(_certInfo, szOID_SUBJECT_ALT_NAME2);
#elif defined(ICE_USE_OPENSSL)
    return convertGeneralNames(reinterpret_cast<GENERAL_NAMES*>(X509_get_ext_d2i(_cert, NID_subject_alt_name, 0, 0)));
#elif defined(ICE_OS_UWP)
    return certificateAltNames(_cert->SubjectAlternativeName);
#else
#   error "Unknown platform"
#endif
}
#endif

int
Certificate::getVersion() const
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    initializeAttributes();
    return _version;
#elif defined(ICE_USE_SECURE_TRANSPORT_MACOS)
    return atoi(getX509String(_cert.get(), kSecOIDX509V1Version).c_str()) - 1;
#elif defined(ICE_USE_SCHANNEL)
    return _certInfo->dwVersion;
#elif defined(ICE_USE_OPENSSL)
    return static_cast<int>(X509_get_version(_cert));
#elif defined(ICE_OS_UWP)
    return -1; // Not supported
#else
#   error "Unknown platform"
#endif
}

string
Certificate::toString() const
{
    ostringstream os;
    os << "serial: " << getSerialNumber() << "\n";
    os << "issuer: " << string(getIssuerDN()) << "\n";
    os << "subject: " << string(getSubjectDN()) << "\n";
#if !defined(ICE_USE_SECURE_TRANSPORT_IOS)

#   ifdef ICE_CPP11_MAPPING
    // Precision is only seconds here, which is probably fine
    os << "notBefore: " << IceUtil::Time::seconds(chrono::system_clock::to_time_t(getNotBefore())).toDateTime() << "\n";
    os << "notAfter: " << IceUtil::Time::seconds(chrono::system_clock::to_time_t(getNotAfter())).toDateTime();
#   else
    os << "notBefore: " << getNotBefore().toDateTime() << "\n";
    os << "notAfter: " << getNotAfter().toDateTime();
#   endif

#endif
    return os.str();
}

X509CertificateRef
Certificate::getCert() const
{
#ifdef __APPLE__
    return _cert.get();
#else
    return _cert;
#endif
}

#if defined(ICE_USE_SECURE_TRANSPORT_IOS)

namespace
{

IceUtil::Mutex* globalMutex = 0;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

}

void
Certificate::initializeAttributes() const
{
    //
    // We need to temporarily add the certificate to the keychain in order to
    // retrieve its attributes. Unfortunately kSecMatchItemList doesn't work
    // on iOS. We make sure only one thread adds/removes a cert at a time here.
    //
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);

    if(_subject)
    {
        return;
    }

    UniqueRef<CFMutableDictionaryRef> query(
        CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    CFDictionarySetValue(query.get(), kSecValueRef, _cert.get());
    CFDictionarySetValue(query.get(), kSecReturnAttributes, kCFBooleanTrue);
    
    UniqueRef<CFDictionaryRef> attributes(0);
    OSStatus err;
    if((err = SecItemAdd(query.get(), reinterpret_cast<CFTypeRef*>(&attributes.get()))) == errSecDuplicateItem)
    {
        CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
        err = SecItemCopyMatching(query.get(), reinterpret_cast<CFTypeRef*>(&attributes.get()));
    }
    else
    {
        query.reset(CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
        CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
        CFDictionarySetValue(query.get(), kSecValueRef, _cert.get());
        err = SecItemDelete(query.get());
    }

    if(err != noErr)
    {
        _subject.reset(0);
        _issuer.reset(0);
        throw CertificateEncodingException(__FILE__, __LINE__, errorToString(err));
    }

    _subject.retain(static_cast<CFDataRef>(CFDictionaryGetValue(attributes.get(), kSecAttrSubject)));
    _issuer.retain(static_cast<CFDataRef>(CFDictionaryGetValue(attributes.get(), kSecAttrIssuer)));
    CFDataRef serial = static_cast<CFDataRef>(CFDictionaryGetValue(attributes.get(), kSecAttrSerialNumber));
    ostringstream os;
    for(int i = 0; i < CFDataGetLength(serial); ++i)
    {
        int c = static_cast<int>(CFDataGetBytePtr(serial)[i]);
        if(i)
        {
            os << ' ';
        }
        os.fill('0');
        os.width(2);
        os << hex << c;
    }
    _serial = os.str();
    CFNumberRef version = static_cast<CFNumberRef>(CFDictionaryGetValue(attributes.get(), kSecAttrCertificateType));
    if(!CFNumberGetValue(version, kCFNumberIntType, &_version))
    {
        _version = -1;
    }
}
#endif
