// Copyright (c) ZeroC, Inc.

// Default to OpenSSL 3.0.0 compatibility (decimal mmnnpp format)
// Hide deprecated APIs unless the user explicitly wants them
#define OPENSSL_API_COMPAT 30000
#define OPENSSL_NO_DEPRECATED

#include "SSLUtil.h"
#include "../Base64.h"
#include "../FileUtil.h"
#include "../Network.h"
#include "../UniqueRef.h"
#include "DistinguishedName.h"
#include "Ice/Config.h"
#include "Ice/SSL/SSLException.h"
#include "Ice/StringUtil.h"
#include "RFC2253.h"

#include <fstream>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace Ice::SSL;

#if defined(__APPLE__)

string
Ice::SSL::fromCFString(CFStringRef v)
{
    string s;
    if (v)
    {
        CFIndex size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(v), kCFStringEncodingUTF8);
        vector<char> buffer;
        buffer.resize(static_cast<size_t>(size + 1));
        CFStringGetCString(v, &buffer[0], static_cast<CFIndex>(buffer.size()), kCFStringEncodingUTF8);
        s.assign(&buffer[0]);
    }
    return s;
}

#endif

bool
Ice::SSL::parseBytes(const string& arg, vector<unsigned char>& buffer)
{
    string v = IceInternal::toUpper(arg);

    // Check for any invalid characters.
    size_t pos = v.find_first_not_of(" :0123456789ABCDEF");
    if (pos != string::npos)
    {
        return false;
    }

    // Remove any separator characters.
    ostringstream s;
    for (char i : v)
    {
        if (i == ' ' || i == ':')
        {
            continue;
        }
        s << i;
    }
    v = s.str();

    // Convert the bytes.
    for (size_t i = 0, length = v.size(); i + 2 <= length;)
    {
        buffer.push_back(static_cast<unsigned char>(strtol(v.substr(i, 2).c_str(), nullptr, 16)));
        i += 2;
    }
    return true;
}

void
Ice::SSL::readFile(const string& file, vector<char>& buffer)
{
    ifstream is(IceInternal::streamFilename(file).c_str(), ios::in | ios::binary);
    if (!is.good())
    {
        throw CertificateReadException(__FILE__, __LINE__, "error opening file " + file);
    }

    is.seekg(0, is.end);
    buffer.resize(static_cast<size_t>(is.tellg()));
    is.seekg(0, is.beg);

    if (!buffer.empty())
    {
        is.read(&buffer[0], static_cast<streamsize>(buffer.size()));
        if (!is.good())
        {
            throw CertificateReadException(__FILE__, __LINE__, "error reading file " + file);
        }
    }
}

optional<string>
Ice::SSL::resolveFilePath(const string& path, const string& parentDir)
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle)
    {
        UniqueRef<CFStringRef> resourceName(toCFString(path));
        UniqueRef<CFStringRef> subDirName(toCFString(parentDir));
        UniqueRef<CFURLRef> url(CFBundleCopyResourceURL(bundle, resourceName.get(), 0, subDirName.get()));

        UInt8 filePath[PATH_MAX];
        if (CFURLGetFileSystemRepresentation(url.get(), true, filePath, sizeof(filePath)))
        {
            string tmp = string(reinterpret_cast<char*>(filePath));
            if (IceInternal::fileExists(tmp))
            {
                return tmp;
            }
        }
    }
#endif
    if (IceInternal::isAbsolutePath(path))
    {
        if (IceInternal::fileExists(path))
        {
            return path;
        }
        return std::nullopt;
    }

    //
    // If a parent directory is provided, the given path is relative to the parent directory.
    //
    string tmp;
    if (!parentDir.empty())
    {
        tmp = parentDir + IceInternal::separator + path;
    }
    else
    {
        tmp = path;
    }

    if (IceInternal::fileExists(tmp))
    {
        return tmp;
    }
    return std::nullopt;
}

optional<string>
Ice::SSL::resolveDirPath(const string& path, const string& parentDir)
{
#if defined(ICE_USE_SECURE_TRANSPORT_IOS)
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle)
    {
        UniqueRef<CFStringRef> resourceName(toCFString(path));
        UniqueRef<CFStringRef> subDirName(toCFString(parentDir));
        UniqueRef<CFURLRef> url(CFBundleCopyResourceURL(bundle, resourceName.get(), 0, subDirName.get()));

        UInt8 filePath[PATH_MAX];
        if (CFURLGetFileSystemRepresentation(url.get(), true, filePath, sizeof(filePath)))
        {
            string tmp = string(reinterpret_cast<char*>(filePath));
            if (IceInternal::directoryExists(tmp))
            {
                return tmp;
            }
        }
    }
#endif
    if (IceInternal::isAbsolutePath(path))
    {
        if (IceInternal::directoryExists(path))
        {
            return path;
        }
        return std::nullopt;
    }

    //
    // If a default directory is provided, the given path is relative to the default directory.
    //
    string tmp;
    if (!parentDir.empty())
    {
        tmp = parentDir + IceInternal::separator + path;
    }
    else
    {
        tmp = path;
    }

    if (IceInternal::directoryExists(tmp))
    {
        return tmp;
    }
    return std::nullopt;
}

namespace
{
    const pair<string, string> certificateOIDS[] = { // NOLINT(cert-err58-cpp)
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
        {"0.9.2342.19200300.100.1.25", "DC"}};
}

#if defined(ICE_USE_SCHANNEL)
namespace
{
    string certNameToString(CERT_NAME_BLOB* certName)
    {
        assert(certName);
        DWORD length = 0;
        if ((length =
                 CertNameToStr(X509_ASN_ENCODING, certName, CERT_OID_NAME_STR | CERT_NAME_STR_REVERSE_FLAG, 0, 0)) == 0)
        {
            throw CertificateEncodingException(__FILE__, __LINE__, IceInternal::lastErrorToString());
        }

        vector<char> buffer(length);
        if (!CertNameToStr(
                X509_ASN_ENCODING,
                certName,
                CERT_OID_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                &buffer[0],
                length))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, IceInternal::lastErrorToString());
        }

        string s(&buffer[0]);
        for (const auto& certificateOID : certificateOIDS)
        {
            const string name = string(certificateOID.first) + "=";
            const string alias = string(certificateOID.second) + "=";
            size_t pos = 0;
            while ((pos = s.find(name, pos)) != string::npos)
            {
                s.replace(pos, name.size(), alias);
            }
        }
        return s;
    }
}

Ice::SSL::ScopedCertificate::~ScopedCertificate()
{
    if (_certificate)
    {
        CertFreeCertificateContext(_certificate);
    }
}

string
Ice::SSL::getSubjectName(PCCERT_CONTEXT cert)
{
    return DistinguishedName(certNameToString(&cert->pCertInfo->Subject));
}

vector<pair<int, string>>
Ice::SSL::getSubjectAltNames(PCCERT_CONTEXT cert)
{
    vector<pair<int, string>> altNames;

    PCERT_EXTENSION extension =
        CertFindExtension(szOID_SUBJECT_ALT_NAME2, cert->pCertInfo->cExtension, cert->pCertInfo->rgExtension);
    if (extension)
    {
        CERT_ALT_NAME_INFO* altName;
        DWORD length = 0;
        if (!CryptDecodeObjectEx(
                X509_ASN_ENCODING,
                X509_ALTERNATE_NAME,
                extension->Value.pbData,
                extension->Value.cbData,
                CRYPT_DECODE_ALLOC_FLAG,
                0,
                &altName,
                &length))
        {
            throw CertificateEncodingException(__FILE__, __LINE__, IceInternal::lastErrorToString());
        }

        for (DWORD i = 0; i < altName->cAltEntry; ++i)
        {
            CERT_ALT_NAME_ENTRY* entry = &altName->rgAltEntry[i];

            switch (entry->dwAltNameChoice)
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
                    if (entry->IPAddress.cbData == 4)
                    {
                        //
                        // IPv4 address
                        //
                        ostringstream os;
                        uint8_t* src = reinterpret_cast<uint8_t*>(entry->IPAddress.pbData);
                        for (int j = 0; j < 4;)
                        {
                            int value = 0;
                            uint8_t* dest = reinterpret_cast<uint8_t*>(&value);
                            *dest = *src++;
                            os << value;
                            if (++j < 4)
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

string
Ice::SSL::encodeCertificate(PCCERT_CONTEXT cert)
{
    string s;
    DWORD encodedLength = 0;
    if (!CryptBinaryToString(
            cert->pbCertEncoded,
            cert->cbCertEncoded,
            CRYPT_STRING_BASE64HEADER | CRYPT_STRING_NOCR,
            0,
            &encodedLength))
    {
        throw CertificateEncodingException(__FILE__, __LINE__, IceInternal::lastErrorToString());
    }

    vector<char> encoded;
    encoded.resize(encodedLength);
    if (!CryptBinaryToString(
            cert->pbCertEncoded,
            cert->cbCertEncoded,
            CRYPT_STRING_BASE64HEADER | CRYPT_STRING_NOCR,
            &encoded[0],
            &encodedLength))
    {
        throw CertificateEncodingException(__FILE__, __LINE__, IceInternal::lastErrorToString());
    }
    s.assign(&encoded[0]);
    return s;
}

PCCERT_CONTEXT
Ice::SSL::decodeCertificate(const string& data)
{
    CERT_SIGNED_CONTENT_INFO* cert = nullptr;
    DWORD derLength = static_cast<DWORD>(data.size());
    vector<BYTE> derBuffer;
    derBuffer.resize(derLength);

    if (!CryptStringToBinary(
            &data.c_str()[0],
            static_cast<DWORD>(data.size()),
            CRYPT_STRING_BASE64HEADER,
            &derBuffer[0],
            &derLength,
            nullptr,
            nullptr))
    {
        // Base64 data should always be bigger than binary
        assert(GetLastError() != ERROR_MORE_DATA);
        throw CertificateEncodingException(__FILE__, __LINE__, IceInternal::lastErrorToString());
    }

    DWORD decodedLeng = 0;
    if (!CryptDecodeObjectEx(
            X509_ASN_ENCODING,
            X509_CERT,
            &derBuffer[0],
            derLength,
            CRYPT_DECODE_ALLOC_FLAG,
            nullptr,
            &cert,
            &decodedLeng))
    {
        throw CertificateEncodingException(__FILE__, __LINE__, IceInternal::lastErrorToString());
    }
    PCCERT_CONTEXT certContext =
        CertCreateCertificateContext(X509_ASN_ENCODING, cert->ToBeSigned.pbData, cert->ToBeSigned.cbData);
    LocalFree(cert);
    if (!certContext)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, IceInternal::lastErrorToString());
    }
    return certContext;
}
#elif defined(ICE_USE_SECURE_TRANSPORT)
string
Ice::SSL::certificateOIDAlias(const string& name)
{
    for (const auto& certificateOID : certificateOIDS)
    {
        if (name == certificateOID.first)
        {
            return certificateOID.second;
        }
    }
    return name;
}

Ice::SSL::ScopedCertificate::~ScopedCertificate()
{
    if (_certificate)
    {
        CFRelease(_certificate);
    }
}
#elif defined(ICE_USE_OPENSSL)

//
// Avoid old style cast warnings from OpenSSL macros
//
#    if defined(__GNUC__)
#        pragma GCC diagnostic ignored "-Wold-style-cast"
#    endif

namespace
{
    string convertX509NameToString(X509_name_st* name)
    {
        BIO* out = BIO_new(BIO_s_mem());
        X509_NAME_print_ex(out, name, 0, XN_FLAG_RFC2253);
        BUF_MEM* p;
        BIO_get_mem_ptr(out, &p);
        string result = string(p->data, p->length);
        BIO_free(out);
        return result;
    }

    vector<pair<int, string>> convertGeneralNames(GENERAL_NAMES* gens)
    {
        vector<pair<int, string>> alt;
        if (gens == nullptr)
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
}

Ice::SSL::ScopedCertificate::~ScopedCertificate()
{
    if (_certificate)
    {
        X509_free(_certificate);
    }
}

string
Ice::SSL::getErrors()
{
    ostringstream ostr;

    unsigned long err;
    bool firstError = true;
    while ((err = ERR_get_error()) != 0)
    {
        if (firstError)
        {
            firstError = false;
        }
        else
        {
            ostr << endl;
        }

        char buf[256];
        ERR_error_string_n(err, buf, sizeof(buf));
        ostr << "error #" << err << ": " << buf;
    }

    return ostr.str();
}

string
Ice::SSL::getSubjectName(X509* certificate)
{
    return DistinguishedName(RFC2253::parseStrict(convertX509NameToString(X509_get_subject_name(certificate))));
}

vector<pair<int, string>>
Ice::SSL::getSubjectAltNames(X509* certificate)
{
    return convertGeneralNames(
        reinterpret_cast<GENERAL_NAMES*>(X509_get_ext_d2i(certificate, NID_subject_alt_name, nullptr, nullptr)));
}

string
Ice::SSL::encodeCertificate(X509* certificate)
{
    BIO* out = BIO_new(BIO_s_mem());
    int i = PEM_write_bio_X509(out, certificate);
    if (i <= 0)
    {
        BIO_free(out);
        throw CertificateEncodingException(__FILE__, __LINE__, getErrors());
    }
    BUF_MEM* p;
    BIO_get_mem_ptr(out, &p);
    string result = string(p->data, p->length);
    BIO_free(out);
    return result;
}

X509*
Ice::SSL::decodeCertificate(const string& data)
{
    BIO* cert = BIO_new_mem_buf(static_cast<void*>(const_cast<char*>(&data[0])), static_cast<int>(data.size()));
    x509_st* x = PEM_read_bio_X509(cert, nullptr, nullptr, nullptr);
    BIO_free(cert);
    if (x == nullptr)
    {
        throw CertificateEncodingException(__FILE__, __LINE__, getErrors());
    }
    // Calling it with -1 for the side effects, this ensure that the extensions info is loaded
    if (X509_check_purpose(x, -1, -1) == -1)
    {
        throw CertificateReadException(__FILE__, __LINE__, "error loading certificate:\n" + getErrors());
    }
    return x;
}
#endif
