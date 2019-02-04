//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceSSL/Plugin.h>
#include <IceSSL/SChannel.h>
#include <IceSSL/CertificateI.h>
#include <IceSSL/Util.h>
#include <Ice/StringUtil.h>
#include <Ice/StringConverter.h>

#include <wincrypt.h>

#include <string>
#include <vector>

using namespace std;
using namespace Ice;
using namespace IceSSL;

namespace
{

class CertInfoHolder : public IceUtil::Shared
{
public:

    CertInfoHolder(CERT_INFO* v) : _certInfo(v)
    {
    }

    virtual ~CertInfoHolder()
    {
        LocalFree(_certInfo);
    }

private:

    CERT_INFO* _certInfo;
};
typedef IceUtil::Handle<CertInfoHolder> CertInfoHolderPtr;

class SCHannelX509ExtensionI : public X509Extension
{

public:

    SCHannelX509ExtensionI(CERT_EXTENSION , const string&, const CertInfoHolderPtr&);
    virtual bool isCritical() const;
    virtual string getOID() const;
    virtual vector<Ice::Byte> getData() const;

private:

    CERT_EXTENSION _extension;
    string _oid;
    CertInfoHolderPtr _certInfo; // Keep a reference on the CERT_INFO struct that holds the extension
};

class SChannelCertificateI : public SChannel::Certificate,
                             public CertificateI,
                             public IceUtil::Mutex
{
public:

    SChannelCertificateI(CERT_SIGNED_CONTENT_INFO*);
    ~SChannelCertificateI();

    virtual bool operator==(const IceSSL::Certificate&) const;

    virtual vector<Ice::Byte> getAuthorityKeyIdentifier() const;
    virtual vector<Ice::Byte> getSubjectKeyIdentifier() const;
    virtual bool verify(const CertificatePtr&) const;
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
    virtual CERT_SIGNED_CONTENT_INFO* getCert() const;

protected:

    virtual void loadX509Extensions() const;

private:

    CERT_SIGNED_CONTENT_INFO* _cert;
    CERT_INFO* _certInfo;
    CertInfoHolderPtr _certInfoHolder;
};

const Ice::Long TICKS_PER_MSECOND = 10000LL;
const Ice::Long MSECS_TO_EPOCH = 11644473600000LL;

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

#  ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#  else
IceUtil::Time
#  endif
filetimeToTime(FILETIME ftime)
{
    Ice::Long value = 0;
    DWORD* dest = reinterpret_cast<DWORD*>(&value);
    *dest++ = ftime.dwLowDateTime;
    *dest = ftime.dwHighDateTime;

    IceUtil::Time time = IceUtil::Time::milliSeconds((value / TICKS_PER_MSECOND) - MSECS_TO_EPOCH);

#  ifdef ICE_CPP11_MAPPING
    return chrono::system_clock::time_point(chrono::microseconds(time.toMicroSeconds()));
#  else
    return time;
#  endif
}

string
certNameToString(CERT_NAME_BLOB* certName)
{
    assert(certName);
    DWORD length = 0;
    if((length = CertNameToStr(X509_ASN_ENCODING, certName, CERT_OID_NAME_STR|CERT_NAME_STR_REVERSE_FLAG, 0, 0)) == 0)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    vector<char> buffer(length);
    if(!CertNameToStr(X509_ASN_ENCODING, certName, CERT_OID_NAME_STR|CERT_NAME_STR_REVERSE_FLAG, &buffer[0], length))
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
                    altNames.push_back(make_pair(AltNameEmail, Ice::wstringToString(entry->pwszRfc822Name)));
                    break;
                }
                case CERT_ALT_NAME_DNS_NAME:
                {
                    altNames.push_back(make_pair(AltNameDNS, Ice::wstringToString(entry->pwszDNSName)));
                    break;
                }
                case CERT_ALT_NAME_DIRECTORY_NAME:
                {
                    altNames.push_back(make_pair(AltNameDirectory, certNameToString(&entry->DirectoryName)));
                    break;
                }
                case CERT_ALT_NAME_URL:
                {
                    altNames.push_back(make_pair(AltNameURL, Ice::wstringToString(entry->pwszURL)));
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

} // End anonymous namespace

SCHannelX509ExtensionI::SCHannelX509ExtensionI(CERT_EXTENSION extension, const string& oid, const CertInfoHolderPtr& ci) :
    _extension(extension),
    _oid(oid),
    _certInfo(ci)
{
}

bool
SCHannelX509ExtensionI::isCritical() const
{
    return _extension.fCritical != 0;
}

string
SCHannelX509ExtensionI::getOID() const
{
    return _oid;
}

vector<Ice::Byte>
SCHannelX509ExtensionI::getData() const
{
    vector<Ice::Byte> data;
    data.resize(_extension.Value.cbData);
    memcpy(&data[0], _extension.Value.pbData, _extension.Value.cbData);
    return data;
}

SChannelCertificateI::SChannelCertificateI(CERT_SIGNED_CONTENT_INFO* cert) :
    _cert(cert)
{
    if(!_cert)
    {
#ifdef ICE_CPP11_MAPPING
        throw invalid_argument("Invalid certificate reference");
#else
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Invalid certificate reference");
#endif
    }

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
        _certInfoHolder = new CertInfoHolder(_certInfo);
    }
    catch(...)
    {
        LocalFree(_cert);
        _cert = 0;
        throw;
    }
}

SChannelCertificateI::~SChannelCertificateI()
{
    if(_cert)
    {
        LocalFree(_cert);
    }
}

bool
SChannelCertificateI::operator==(const IceSSL::Certificate& r) const
{
    const SChannelCertificateI* p = dynamic_cast<const SChannelCertificateI*>(&r);
    if(!p)
    {
        return false;
    }

    return CertCompareCertificate(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, _certInfo, p->_certInfo) != 0;
}

vector<Ice::Byte>
SChannelCertificateI::getAuthorityKeyIdentifier() const
{
    vector<Ice::Byte> keyid;
    PCERT_EXTENSION extension = CertFindExtension(szOID_AUTHORITY_KEY_IDENTIFIER2, _certInfo->cExtension,
                                                  _certInfo->rgExtension);
    if(extension)
    {
        CERT_AUTHORITY_KEY_ID2_INFO* decoded;
        DWORD length = 0;
        if(!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_AUTHORITY_KEY_ID2, extension->Value.pbData,
                                extension->Value.cbData, CRYPT_DECODE_ALLOC_FLAG, 0, &decoded, &length))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
        }

        if(decoded->KeyId.pbData && decoded->KeyId.cbData)
        {
            keyid.resize(decoded->KeyId.cbData);
            memcpy(&keyid[0], decoded->KeyId.pbData, decoded->KeyId.cbData);
            LocalFree(decoded);
        }
    }
    return keyid;
}

vector<Ice::Byte>
SChannelCertificateI::getSubjectKeyIdentifier() const
{
    vector<Ice::Byte> keyid;
    PCERT_EXTENSION extension = CertFindExtension(szOID_SUBJECT_KEY_IDENTIFIER, _certInfo->cExtension,
                                                  _certInfo->rgExtension);
    if(extension)
    {
        CRYPT_DATA_BLOB* decoded;
        DWORD length = 0;
        if(!CryptDecodeObjectEx(X509_ASN_ENCODING, szOID_SUBJECT_KEY_IDENTIFIER, extension->Value.pbData,
                                extension->Value.cbData, CRYPT_DECODE_ALLOC_FLAG, 0, &decoded, &length))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
        }

        if(decoded->pbData && decoded->cbData)
        {
            keyid.resize(decoded->cbData);
            memcpy(&keyid[0], decoded->pbData, decoded->cbData);
            LocalFree(decoded);
        }
    }
    return keyid;
}

bool
SChannelCertificateI::verify(const CertificatePtr& cert) const
{
    bool result = false;
    SChannelCertificateI* c = dynamic_cast<SChannelCertificateI*>(cert.get());
    if(c)
    {
        BYTE* buffer = 0;
        DWORD length = 0;
        if(!CryptEncodeObjectEx(X509_ASN_ENCODING, X509_CERT, _cert, CRYPT_ENCODE_ALLOC_FLAG , 0, &buffer, &length))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
        }
        result = CryptVerifyCertificateSignature(0, X509_ASN_ENCODING, buffer, length, &c->_certInfo->SubjectPublicKeyInfo) != 0;
        LocalFree(buffer);
    }
    return result;
}

string
SChannelCertificateI::encode() const
{
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
}

#  ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#  else
IceUtil::Time
#  endif
SChannelCertificateI::getNotAfter() const
{
    return filetimeToTime(_certInfo->NotAfter);
}

#  ifdef ICE_CPP11_MAPPING
chrono::system_clock::time_point
#  else
IceUtil::Time
#  endif
SChannelCertificateI::getNotBefore() const
{
    return filetimeToTime(_certInfo->NotBefore);
}

string
SChannelCertificateI::getSerialNumber() const
{
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
}

DistinguishedName
SChannelCertificateI::getIssuerDN() const
{
    return DistinguishedName(certNameToString(&_certInfo->Issuer));
}

vector<pair<int, string> >
SChannelCertificateI::getIssuerAlternativeNames() const
{
    return certificateAltNames(_certInfo, szOID_ISSUER_ALT_NAME2);
}

DistinguishedName
SChannelCertificateI::getSubjectDN() const
{
    return DistinguishedName(certNameToString(&_certInfo->Subject));
}

vector<pair<int, string> >
SChannelCertificateI::getSubjectAlternativeNames() const
{
    return certificateAltNames(_certInfo, szOID_SUBJECT_ALT_NAME2);
}

int
SChannelCertificateI::getVersion() const
{
    return _certInfo->dwVersion;
}

CERT_SIGNED_CONTENT_INFO*
SChannelCertificateI::getCert() const
{
    return _cert;
}

void
SChannelCertificateI::loadX509Extensions() const
{
    IceUtil::Mutex::Lock sync(*this);
    if(_extensions.empty())
    {
        for(size_t i = 0; i < _certInfo->cExtension; ++i)
        {
            CERT_EXTENSION ext = _certInfo->rgExtension[i];
            _extensions.push_back(ICE_DYNAMIC_CAST(X509Extension,
                ICE_MAKE_SHARED(SCHannelX509ExtensionI, ext, ext.pszObjId,
                _certInfoHolder)));
        }
    }
}

SChannel::CertificatePtr
SChannel::Certificate::create(CERT_SIGNED_CONTENT_INFO* cert)
{
    return ICE_MAKE_SHARED(SChannelCertificateI, cert);
}

SChannel::CertificatePtr
SChannel::Certificate::load(const std::string& file)
{
    CERT_SIGNED_CONTENT_INFO* cert;
    loadCertificate(&cert, file);
    return ICE_MAKE_SHARED(SChannelCertificateI, cert);
}

SChannel::CertificatePtr
SChannel::Certificate::decode(const std::string& encoding)
{
    CERT_SIGNED_CONTENT_INFO* cert;
    loadCertificate(&cert, encoding.c_str(), static_cast<DWORD>(encoding.size()));
    return ICE_MAKE_SHARED(SChannelCertificateI, cert);
}
