//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// Disable deprecation warnings for SecCertificateCopyNormalizedIssuerContent and
// SecCertificateCopyNormalizedSubjectContent
//
#include <IceUtil/DisableWarnings.h>

#include <IceSSL/Plugin.h>
#include <IceSSL/SecureTransport.h>
#include <IceSSL/CertificateI.h>
#include <IceSSL/SecureTransportUtil.h>
#include <IceSSL/RFC2253.h>

#include <Ice/LocalException.h>
#include <Ice/UniqueRef.h>
#include <Ice/Base64.h>

#include <IceUtil/MutexPtrLock.h>

#include <Security/Security.h>

using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;
using namespace IceSSL::SecureTransport;

using namespace std;

namespace
{

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

#ifdef ICE_USE_SECURE_TRANSPORT_IOS
//
// ASN1Parser to pase the subject/issuer ASN.1 DER encoded attributes on iOS.
//
class ASN1Parser
{
public:

    ASN1Parser(CFDataRef data) :
        _data(CFDataGetBytePtr(data)),
        _length(static_cast<size_t>(CFDataGetLength(data))),
        _p(_data),
        _next(0)
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
        string v(reinterpret_cast<const char*>(_p), static_cast<size_t>(length));
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

class SecureTransportCertificateI ICE_FINAL : public IceSSL::SecureTransport::Certificate,
                                              public IceSSL::CertificateI
{
public:

    SecureTransportCertificateI(SecCertificateRef);

    virtual bool operator==(const IceSSL::Certificate&) const;

    virtual vector<Ice::Byte> getAuthorityKeyIdentifier() const;
    virtual vector<Ice::Byte> getSubjectKeyIdentifier() const;
    virtual bool verify(const IceSSL::CertificatePtr&) const;
    virtual string encode() const;

#ifdef ICE_CPP11_MAPPING
    virtual chrono::system_clock::time_point getNotAfter() const;
    virtual chrono::system_clock::time_point getNotBefore() const;
#else
    virtual IceUtil::Time getNotAfter() const;
    virtual IceUtil::Time getNotBefore() const;
#endif

    virtual string getSerialNumber() const;
    virtual DistinguishedName getIssuerDN() const;
    virtual vector<pair<int, string> > getIssuerAlternativeNames() const;
    virtual DistinguishedName getSubjectDN() const;
    virtual vector<pair<int, string> > getSubjectAlternativeNames() const;
    virtual int getVersion() const;
    virtual SecCertificateRef getCert() const;

private:

    IceInternal::UniqueRef<SecCertificateRef> _cert;

#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    void initializeAttributes() const;

    mutable IceInternal::UniqueRef<CFDataRef> _subject;
    mutable IceInternal::UniqueRef<CFDataRef> _issuer;
    mutable std::string _serial;
    mutable int _version;
#endif
};

#ifndef ICE_USE_SECURE_TRANSPORT_IOS

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
        CFIndex size = CFArrayGetCount(dn);
        for(CFIndex i = 0; i < size; ++i)
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
        CFIndex size = CFArrayGetCount(names);

        for(CFIndex i = 0; i < size; ++i)
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
                    for(CFIndex j = 0, count = CFArrayGetCount(section); j < count;)
                    {
                        CFDictionaryRef d = (CFDictionaryRef)CFArrayGetValueAtIndex(section, j);

                        CFStringRef sectionLabel = static_cast<CFStringRef>(CFDictionaryGetValue(d, kSecPropertyKeyLabel));
                        CFStringRef sectionValue = static_cast<CFStringRef>(CFDictionaryGetValue(d, kSecPropertyKeyValue));

                        os << certificateOIDAlias(fromCFString(sectionLabel)) << "=" << fromCFString(sectionValue);
                        if(++j < count)
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
#endif

} // end anonymous namespace

SecureTransportCertificateI::SecureTransportCertificateI(SecCertificateRef cert) : _cert(cert)
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

bool
SecureTransportCertificateI::operator==(const IceSSL::Certificate& r) const
{
    const SecureTransportCertificateI* p = dynamic_cast<const SecureTransportCertificateI*>(&r);
    if(!p)
    {
        return false;
    }
    return CFEqual(_cert.get(), p->_cert.get());
}

vector<Ice::Byte>
SecureTransportCertificateI::getAuthorityKeyIdentifier() const
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    throw Ice::FeatureNotSupportedException(__FILE__, __LINE__);
#else // macOS
    vector<Ice::Byte> keyid;

    UniqueRef<CFDictionaryRef> property(getCertificateProperty(_cert.get(), kSecOIDAuthorityKeyIdentifier));
    if(property)
    {
        CFTypeRef type = 0;
        CFTypeRef value = 0;
        if(CFDictionaryGetValueIfPresent(property.get(), kSecPropertyKeyType, &type))
        {
            if(CFEqual(type, kSecPropertyTypeSection))
            {
                if(CFDictionaryGetValueIfPresent(property.get(), kSecPropertyKeyValue, &value))
                {
                    if(CFArrayGetCount(static_cast<CFArrayRef>(value)) >= 0)
                    {
                        value = CFArrayGetValueAtIndex(static_cast<CFArrayRef>(value), 1);
                        type = CFDictionaryGetValue(static_cast<CFDictionaryRef>(value), kSecPropertyKeyType);
                    }
                }
            }

            if(CFEqual(type, kSecPropertyTypeData))
            {
                CFDataRef data = static_cast<CFDataRef>(
                    CFDictionaryGetValue(static_cast<CFDictionaryRef>(value), kSecPropertyKeyValue));
                keyid.resize(static_cast<size_t>(CFDataGetLength(data)));
                memcpy(&keyid[0], CFDataGetBytePtr(data), static_cast<size_t>(CFDataGetLength(data)));
            }
        }
    }
    return keyid;
#endif
}

vector<Ice::Byte>
SecureTransportCertificateI::getSubjectKeyIdentifier() const
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    throw Ice::FeatureNotSupportedException(__FILE__, __LINE__);
#else // macOS
    vector<Ice::Byte> keyid;
    UniqueRef<CFDictionaryRef> property(getCertificateProperty(_cert.get(), kSecOIDSubjectKeyIdentifier));
    if(property)
    {
        CFTypeRef type = 0;
        CFTypeRef value = 0;
        if(CFDictionaryGetValueIfPresent(property.get(), kSecPropertyKeyType, &type))
        {
            if(CFEqual(type, kSecPropertyTypeSection))
            {
                if(CFDictionaryGetValueIfPresent(property.get(), kSecPropertyKeyValue, &value))
                {
                    if(CFArrayGetCount(static_cast<CFArrayRef>(value)) >= 0)
                    {
                        value = CFArrayGetValueAtIndex(static_cast<CFArrayRef>(value), 1);
                        type = CFDictionaryGetValue(static_cast<CFDictionaryRef>(value), kSecPropertyKeyType);
                    }
                }
            }

            if(CFEqual(type, kSecPropertyTypeData))
            {
                CFDataRef data = static_cast<CFDataRef>(
                    CFDictionaryGetValue(static_cast<CFDictionaryRef>(value), kSecPropertyKeyValue));
                keyid.resize(static_cast<size_t>(CFDataGetLength(data)));
                memcpy(&keyid[0], CFDataGetBytePtr(data), static_cast<size_t>(CFDataGetLength(data)));
            }
        }
    }
    return keyid;
#endif
}

bool
SecureTransportCertificateI::verify(const IceSSL::CertificatePtr& cert) const
{
    bool valid = false;
    SecureTransportCertificateI* c = dynamic_cast<SecureTransportCertificateI*>(cert.get());
    if(c)
    {
        //
        // We first check if the given certificate subject match our certificate
        // issuer. Otherwhise when checking a certificate against itself
        // SecTrustEvaluate always returns it is valid.
        //
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
        initializeAttributes();
        c->initializeAttributes();
        valid = CFEqual(_issuer.get(), c->_subject.get());
#else // macOS
        UniqueRef<CFErrorRef> error;
        UniqueRef<CFDataRef> issuer(SecCertificateCopyNormalizedIssuerContent(_cert.get(), &error.get()));
        if(error)
        {
            throw CertificateEncodingException(__FILE__, __LINE__, "certificate error:\n" + sslErrorToString(error.get()));
        }
        UniqueRef<CFDataRef> subject(SecCertificateCopyNormalizedSubjectContent(c->getCert(), &error.get()));
        if(error)
        {
            throw CertificateEncodingException(__FILE__, __LINE__, "certificate error:\n" + sslErrorToString(error.get()));
        }

        //
        // The certificate issuer must match the CA subject.
        //
        valid = CFEqual(issuer.get(), subject.get());
#endif
        if(valid)
        {
            UniqueRef<SecPolicyRef> policy(SecPolicyCreateBasicX509());
            UniqueRef<SecTrustRef> trust;
            OSStatus err = 0;
            if((err = SecTrustCreateWithCertificates(_cert.get(), policy.get(), &trust.get())))
            {
                throw CertificateEncodingException(__FILE__, __LINE__, sslErrorToString(err));
            }

            SecCertificateRef certs[1] = { c->getCert() };
            UniqueRef<CFArrayRef> anchorCertificates(
                CFArrayCreate(kCFAllocatorDefault, (const void**)&certs, 1, &kCFTypeArrayCallBacks));
            if((err = SecTrustSetAnchorCertificates(trust.get(), anchorCertificates.get())))
            {
                throw CertificateEncodingException(__FILE__, __LINE__,  sslErrorToString(err));
            }

            SecTrustResultType trustResult = kSecTrustResultInvalid;
            if((err = SecTrustEvaluate(trust.get(), &trustResult)))
            {
                throw CertificateEncodingException(__FILE__, __LINE__,  sslErrorToString(err));
            }

            valid = trustResult == kSecTrustResultUnspecified;
        }
    }
    return valid;
}

string
SecureTransportCertificateI::encode() const
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    UniqueRef<CFDataRef> cert(SecCertificateCopyData(_cert.get()));
    vector<unsigned char> data(CFDataGetBytePtr(cert.get()), CFDataGetBytePtr(cert.get()) + CFDataGetLength(cert.get()));
    ostringstream os;
    os << "-----BEGIN CERTIFICATE-----\n";
    os << IceInternal::Base64::encode(data);
    os << "\n-----END CERTIFICATE-----\n";
    return os.str();
#else // macOS
    UniqueRef<CFDataRef> exported;
    OSStatus err = SecItemExport(_cert.get(), kSecFormatPEMSequence, kSecItemPemArmour, 0, &exported.get());
    if(err != noErr)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, sslErrorToString(err));
    }
    return string(reinterpret_cast<const char*>(CFDataGetBytePtr(exported.get())),
                  static_cast<size_t>(CFDataGetLength(exported.get())));
#endif
}

#ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#else
IceUtil::Time
#endif
SecureTransportCertificateI::getNotAfter() const
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    throw Ice::FeatureNotSupportedException(__FILE__, __LINE__);
#else // macOS
    return getX509Date(_cert.get(), kSecOIDX509V1ValidityNotAfter);
#endif
}

#ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#else
IceUtil::Time
#endif
SecureTransportCertificateI::getNotBefore() const
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    throw Ice::FeatureNotSupportedException(__FILE__, __LINE__);
#else // macOS
    return getX509Date(_cert.get(), kSecOIDX509V1ValidityNotBefore);
#endif
}

string
SecureTransportCertificateI::getSerialNumber() const
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    initializeAttributes();
    return _serial;
#else // macOS
    return getX509String(_cert.get(), kSecOIDX509V1SerialNumber);
#endif
}

DistinguishedName
SecureTransportCertificateI::getIssuerDN() const
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    initializeAttributes();
    return _issuer ? DistinguishedName(ASN1Parser(_issuer.get()).parse()) : DistinguishedName("");
#else // macOS
    return getX509Name(_cert.get(), kSecOIDX509V1IssuerName);
#endif
}

vector<pair<int, string> >
SecureTransportCertificateI::getIssuerAlternativeNames() const
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    throw FeatureNotSupportedException(__FILE__, __LINE__);
#else // macOS
    return getX509AltName(_cert.get(), kSecOIDIssuerAltName);
#endif
}

DistinguishedName
SecureTransportCertificateI::getSubjectDN() const
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    initializeAttributes();
    if(_subject)
    {
        return DistinguishedName(ASN1Parser(_subject.get()).parse());
    }
    else
    {
        UniqueRef<CFStringRef> subjectSummary(SecCertificateCopySubjectSummary(_cert.get()));
        return DistinguishedName("CN=" + fromCFString(subjectSummary.get()));
    }
#else // macOS
    return getX509Name(_cert.get(), kSecOIDX509V1SubjectName);
#endif
}

vector<pair<int, string> >
SecureTransportCertificateI::getSubjectAlternativeNames() const
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    throw FeatureNotSupportedException(__FILE__, __LINE__);
#else // macOS
    return getX509AltName(_cert.get(), kSecOIDSubjectAltName);
#endif
}

int
SecureTransportCertificateI::getVersion() const
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    initializeAttributes();
    return _version;
#else // macOS
    return atoi(getX509String(_cert.get(), kSecOIDX509V1Version).c_str()) - 1;
#endif
}

SecCertificateRef
SecureTransportCertificateI::getCert() const
{
    return _cert.get();
}

#ifdef ICE_USE_SECURE_TRANSPORT_IOS

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

void
SecureTransportCertificateI::initializeAttributes() const
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
        throw CertificateEncodingException(__FILE__, __LINE__, sslErrorToString(err));
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

IceSSL::SecureTransport::CertificatePtr
IceSSL::SecureTransport::Certificate::create(SecCertificateRef cert)
{
    return ICE_MAKE_SHARED(SecureTransportCertificateI, cert);
}

IceSSL::SecureTransport::CertificatePtr
IceSSL::SecureTransport::Certificate::load(const std::string& file)
{
    string resolved;
    if(checkPath(file, "", false, resolved))
    {
        return ICE_MAKE_SHARED(SecureTransportCertificateI, loadCertificate(resolved));
    }
    else
    {
        throw CertificateReadException(__FILE__, __LINE__, "error opening file " + file);
    }
}

IceSSL::SecureTransport::CertificatePtr
IceSSL::SecureTransport::Certificate::decode(const std::string& encoding)
{
#ifdef ICE_USE_SECURE_TRANSPORT_IOS
    string::size_type size = 0;
    string::size_type startpos = 0;
    startpos = encoding.find("-----BEGIN CERTIFICATE-----", 0);
    if(startpos != string::npos)
    {
        startpos += sizeof("-----BEGIN CERTIFICATE-----");
        string::size_type endpos = encoding.find("-----END CERTIFICATE-----", startpos);
        size = endpos - startpos;
    }
    else
    {
        startpos = 0;
        size = encoding.size();
    }

    vector<unsigned char> data(IceInternal::Base64::decode(string(&encoding[startpos], size)));
    UniqueRef<CFDataRef> certdata(CFDataCreate(kCFAllocatorDefault, &data[0], static_cast<CFIndex>(data.size())));
    SecCertificateRef cert = SecCertificateCreateWithData(0, certdata.get());
    if(!cert)
    {
        assert(false);
        throw CertificateEncodingException(__FILE__, __LINE__, "certificate is not a valid PEM-encoded certificate");
    }
    return ICE_MAKE_SHARED(SecureTransportCertificateI, cert);
#else // macOS
    UniqueRef<CFDataRef> data(
        CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
                                    reinterpret_cast<const UInt8*>(encoding.c_str()),
                                    static_cast<CFIndex>(encoding.size()), kCFAllocatorNull));

    SecExternalFormat format = kSecFormatUnknown;
    SecExternalItemType type = kSecItemTypeCertificate;

    SecItemImportExportKeyParameters params;
    memset(&params, 0, sizeof(params));
    params.version =  SEC_KEY_IMPORT_EXPORT_PARAMS_VERSION;

    UniqueRef<CFArrayRef> items;
    OSStatus err = SecItemImport(data.get(), 0, &format, &type, 0, &params, 0, &items.get());
    if(err)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, sslErrorToString(err));
    }

    UniqueRef<SecKeychainItemRef> item;
    item.retain(static_cast<SecKeychainItemRef>(const_cast<void*>(CFArrayGetValueAtIndex(items.get(), 0))));
    assert(SecCertificateGetTypeID() == CFGetTypeID(item.get()));
    return ICE_MAKE_SHARED(SecureTransportCertificateI, reinterpret_cast<SecCertificateRef>(item.release()));
#endif
}
