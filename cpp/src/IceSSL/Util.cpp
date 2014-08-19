// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Config.h>
#ifdef _WIN32
#   include <winsock2.h>
#endif

#include <IceSSL/Util.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/StringUtil.h>

#include <Ice/LocalException.h>
#include <Ice/Network.h>
#include <Ice/Object.h>

#ifdef ICE_USE_OPENSSL
#   include <openssl/err.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceUtil;
using namespace IceSSL;

void
IceSSL::readFile(const string& file, vector<char>& buffer)
{
    IceUtilInternal::ifstream is(file, ios::in | ios::binary);
    if(!is.good())
    {
        throw CertificateReadException(__FILE__, __LINE__, "error opening file " + file);
    }
    
    is.seekg(0, is.end);
    buffer.resize(static_cast<int>(is.tellg()));
    is.seekg(0, is.beg);
    
    is.read(&buffer[0], buffer.size());
    
    if(!is.good())
    {
        throw CertificateReadException(__FILE__, __LINE__, "error reading file " + file);
    }
}

namespace
{
bool
parseBytes(const string& arg, vector<unsigned char>& buffer)
{
    string v = IceUtilInternal::toUpper(arg);

    //
    // Check for any invalid characters.
    //
    size_t pos = v.find_first_not_of(" :0123456789ABCDEF");
    if(pos != string::npos)
    {
        return false;
    }

    //
    // Remove any separator characters.
    //
    ostringstream s;
    for(string::const_iterator i = v.begin(); i != v.end(); ++i)
    {
        if(*i == ' ' || *i == ':')
        {
            continue;
        }
        s << *i;
    }
    v = s.str();

    //
    // Convert the bytes.
    //
    for(size_t i = 0, length = v.size(); i + 2 <= length;)
    {
        buffer.push_back(static_cast<unsigned char>(strtol(v.substr(i, 2).c_str(), 0, 16)));
        i += 2;
    }
    return true;
}
}

#ifdef ICE_USE_OPENSSL
namespace
{

#   ifndef OPENSSL_NO_DH

// The following arrays are predefined Diffie Hellman group parameters.
// These are known strong primes, distributed with the OpenSSL library
// in the files dh512.pem, dh1024.pem, dh2048.pem and dh4096.pem.
// They are not keys themselves, but the basis for generating DH keys
// on the fly.

unsigned char dh512_p[] =
{
    0xF5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,0x18,0x11,0x8D,0x7C,
    0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,0x19,0xC8,0x07,0x29,
    0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,0xD0,0x0A,0x50,0x9B,
    0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,0x41,0x9F,0x9C,0x7C,
    0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,0xA2,0x5E,0xC3,0x55,
    0xE9,0x2A,0x05,0x5F,
};

unsigned char dh512_g[] = { 0x02 };

unsigned char dh1024_p[] =
{
    0xF4,0x88,0xFD,0x58,0x4E,0x49,0xDB,0xCD,0x20,0xB4,0x9D,0xE4,
    0x91,0x07,0x36,0x6B,0x33,0x6C,0x38,0x0D,0x45,0x1D,0x0F,0x7C,
    0x88,0xB3,0x1C,0x7C,0x5B,0x2D,0x8E,0xF6,0xF3,0xC9,0x23,0xC0,
    0x43,0xF0,0xA5,0x5B,0x18,0x8D,0x8E,0xBB,0x55,0x8C,0xB8,0x5D,
    0x38,0xD3,0x34,0xFD,0x7C,0x17,0x57,0x43,0xA3,0x1D,0x18,0x6C,
    0xDE,0x33,0x21,0x2C,0xB5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,
    0x18,0x11,0x8D,0x7C,0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,
    0x19,0xC8,0x07,0x29,0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,
    0xD0,0x0A,0x50,0x9B,0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,
    0x41,0x9F,0x9C,0x7C,0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,
    0xA2,0x5E,0xC3,0x55,0xE9,0x2F,0x78,0xC7,
};

unsigned char dh1024_g[] = { 0x02 };

unsigned char dh2048_p[] =
{
    0xF6,0x42,0x57,0xB7,0x08,0x7F,0x08,0x17,0x72,0xA2,0xBA,0xD6,
    0xA9,0x42,0xF3,0x05,0xE8,0xF9,0x53,0x11,0x39,0x4F,0xB6,0xF1,
    0x6E,0xB9,0x4B,0x38,0x20,0xDA,0x01,0xA7,0x56,0xA3,0x14,0xE9,
    0x8F,0x40,0x55,0xF3,0xD0,0x07,0xC6,0xCB,0x43,0xA9,0x94,0xAD,
    0xF7,0x4C,0x64,0x86,0x49,0xF8,0x0C,0x83,0xBD,0x65,0xE9,0x17,
    0xD4,0xA1,0xD3,0x50,0xF8,0xF5,0x59,0x5F,0xDC,0x76,0x52,0x4F,
    0x3D,0x3D,0x8D,0xDB,0xCE,0x99,0xE1,0x57,0x92,0x59,0xCD,0xFD,
    0xB8,0xAE,0x74,0x4F,0xC5,0xFC,0x76,0xBC,0x83,0xC5,0x47,0x30,
    0x61,0xCE,0x7C,0xC9,0x66,0xFF,0x15,0xF9,0xBB,0xFD,0x91,0x5E,
    0xC7,0x01,0xAA,0xD3,0x5B,0x9E,0x8D,0xA0,0xA5,0x72,0x3A,0xD4,
    0x1A,0xF0,0xBF,0x46,0x00,0x58,0x2B,0xE5,0xF4,0x88,0xFD,0x58,
    0x4E,0x49,0xDB,0xCD,0x20,0xB4,0x9D,0xE4,0x91,0x07,0x36,0x6B,
    0x33,0x6C,0x38,0x0D,0x45,0x1D,0x0F,0x7C,0x88,0xB3,0x1C,0x7C,
    0x5B,0x2D,0x8E,0xF6,0xF3,0xC9,0x23,0xC0,0x43,0xF0,0xA5,0x5B,
    0x18,0x8D,0x8E,0xBB,0x55,0x8C,0xB8,0x5D,0x38,0xD3,0x34,0xFD,
    0x7C,0x17,0x57,0x43,0xA3,0x1D,0x18,0x6C,0xDE,0x33,0x21,0x2C,
    0xB5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,0x18,0x11,0x8D,0x7C,
    0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,0x19,0xC8,0x07,0x29,
    0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,0xD0,0x0A,0x50,0x9B,
    0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,0x41,0x9F,0x9C,0x7C,
    0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,0xA2,0x5E,0xC3,0x55,
    0xE9,0x32,0x0B,0x3B,
};

unsigned char dh2048_g[] = { 0x02 };

unsigned char dh4096_p[] =
{
    0xFA,0x14,0x72,0x52,0xC1,0x4D,0xE1,0x5A,0x49,0xD4,0xEF,0x09,
    0x2D,0xC0,0xA8,0xFD,0x55,0xAB,0xD7,0xD9,0x37,0x04,0x28,0x09,
    0xE2,0xE9,0x3E,0x77,0xE2,0xA1,0x7A,0x18,0xDD,0x46,0xA3,0x43,
    0x37,0x23,0x90,0x97,0xF3,0x0E,0xC9,0x03,0x50,0x7D,0x65,0xCF,
    0x78,0x62,0xA6,0x3A,0x62,0x22,0x83,0xA1,0x2F,0xFE,0x79,0xBA,
    0x35,0xFF,0x59,0xD8,0x1D,0x61,0xDD,0x1E,0x21,0x13,0x17,0xFE,
    0xCD,0x38,0x87,0x9E,0xF5,0x4F,0x79,0x10,0x61,0x8D,0xD4,0x22,
    0xF3,0x5A,0xED,0x5D,0xEA,0x21,0xE9,0x33,0x6B,0x48,0x12,0x0A,
    0x20,0x77,0xD4,0x25,0x60,0x61,0xDE,0xF6,0xB4,0x4F,0x1C,0x63,
    0x40,0x8B,0x3A,0x21,0x93,0x8B,0x79,0x53,0x51,0x2C,0xCA,0xB3,
    0x7B,0x29,0x56,0xA8,0xC7,0xF8,0xF4,0x7B,0x08,0x5E,0xA6,0xDC,
    0xA2,0x45,0x12,0x56,0xDD,0x41,0x92,0xF2,0xDD,0x5B,0x8F,0x23,
    0xF0,0xF3,0xEF,0xE4,0x3B,0x0A,0x44,0xDD,0xED,0x96,0x84,0xF1,
    0xA8,0x32,0x46,0xA3,0xDB,0x4A,0xBE,0x3D,0x45,0xBA,0x4E,0xF8,
    0x03,0xE5,0xDD,0x6B,0x59,0x0D,0x84,0x1E,0xCA,0x16,0x5A,0x8C,
    0xC8,0xDF,0x7C,0x54,0x44,0xC4,0x27,0xA7,0x3B,0x2A,0x97,0xCE,
    0xA3,0x7D,0x26,0x9C,0xAD,0xF4,0xC2,0xAC,0x37,0x4B,0xC3,0xAD,
    0x68,0x84,0x7F,0x99,0xA6,0x17,0xEF,0x6B,0x46,0x3A,0x7A,0x36,
    0x7A,0x11,0x43,0x92,0xAD,0xE9,0x9C,0xFB,0x44,0x6C,0x3D,0x82,
    0x49,0xCC,0x5C,0x6A,0x52,0x42,0xF8,0x42,0xFB,0x44,0xF9,0x39,
    0x73,0xFB,0x60,0x79,0x3B,0xC2,0x9E,0x0B,0xDC,0xD4,0xA6,0x67,
    0xF7,0x66,0x3F,0xFC,0x42,0x3B,0x1B,0xDB,0x4F,0x66,0xDC,0xA5,
    0x8F,0x66,0xF9,0xEA,0xC1,0xED,0x31,0xFB,0x48,0xA1,0x82,0x7D,
    0xF8,0xE0,0xCC,0xB1,0xC7,0x03,0xE4,0xF8,0xB3,0xFE,0xB7,0xA3,
    0x13,0x73,0xA6,0x7B,0xC1,0x0E,0x39,0xC7,0x94,0x48,0x26,0x00,
    0x85,0x79,0xFC,0x6F,0x7A,0xAF,0xC5,0x52,0x35,0x75,0xD7,0x75,
    0xA4,0x40,0xFA,0x14,0x74,0x61,0x16,0xF2,0xEB,0x67,0x11,0x6F,
    0x04,0x43,0x3D,0x11,0x14,0x4C,0xA7,0x94,0x2A,0x39,0xA1,0xC9,
    0x90,0xCF,0x83,0xC6,0xFF,0x02,0x8F,0xA3,0x2A,0xAC,0x26,0xDF,
    0x0B,0x8B,0xBE,0x64,0x4A,0xF1,0xA1,0xDC,0xEE,0xBA,0xC8,0x03,
    0x82,0xF6,0x62,0x2C,0x5D,0xB6,0xBB,0x13,0x19,0x6E,0x86,0xC5,
    0x5B,0x2B,0x5E,0x3A,0xF3,0xB3,0x28,0x6B,0x70,0x71,0x3A,0x8E,
    0xFF,0x5C,0x15,0xE6,0x02,0xA4,0xCE,0xED,0x59,0x56,0xCC,0x15,
    0x51,0x07,0x79,0x1A,0x0F,0x25,0x26,0x27,0x30,0xA9,0x15,0xB2,
    0xC8,0xD4,0x5C,0xCC,0x30,0xE8,0x1B,0xD8,0xD5,0x0F,0x19,0xA8,
    0x80,0xA4,0xC7,0x01,0xAA,0x8B,0xBA,0x53,0xBB,0x47,0xC2,0x1F,
    0x6B,0x54,0xB0,0x17,0x60,0xED,0x79,0x21,0x95,0xB6,0x05,0x84,
    0x37,0xC8,0x03,0xA4,0xDD,0xD1,0x06,0x69,0x8F,0x4C,0x39,0xE0,
    0xC8,0x5D,0x83,0x1D,0xBE,0x6A,0x9A,0x99,0xF3,0x9F,0x0B,0x45,
    0x29,0xD4,0xCB,0x29,0x66,0xEE,0x1E,0x7E,0x3D,0xD7,0x13,0x4E,
    0xDB,0x90,0x90,0x58,0xCB,0x5E,0x9B,0xCD,0x2E,0x2B,0x0F,0xA9,
    0x4E,0x78,0xAC,0x05,0x11,0x7F,0xE3,0x9E,0x27,0xD4,0x99,0xE1,
    0xB9,0xBD,0x78,0xE1,0x84,0x41,0xA0,0xDF,
};

unsigned char dh4096_g[] = { 0x02 };

}

//
// Convert a predefined parameter set into a DH value.
//
static DH*
convertDH(unsigned char* p, int plen, unsigned char* g, int glen)
{
    assert(p != 0);
    assert(g != 0);

    DH* dh = DH_new();

    if(dh != 0)
    {
        dh->p = BN_bin2bn(p, plen, 0);
        dh->g = BN_bin2bn(g, glen, 0);

        if((dh->p == 0) || (dh->g == 0))
        {
            DH_free(dh);
            dh = 0;
        }
    }

    return dh;
}

IceSSL::DHParams::DHParams() :
    _dh512(0), _dh1024(0), _dh2048(0), _dh4096(0)
{
}

IceSSL::DHParams::~DHParams()
{
    for(ParamList::iterator p = _params.begin(); p != _params.end(); ++p)
    {
        DH_free(p->second);
    }
    DH_free(_dh512);
    DH_free(_dh1024);
    DH_free(_dh2048);
    DH_free(_dh4096);
}

bool
IceSSL::DHParams::add(int keyLength, const string& file)
{
    BIO* bio = BIO_new(BIO_s_file());
    if(BIO_read_filename(bio, file.c_str()) <= 0)
    {
        BIO_free(bio);
        return false;
    }
    DH* dh = PEM_read_bio_DHparams(bio, 0, 0, 0);
    BIO_free(bio);
    if(!dh)
    {
        return false;
    }
    ParamList::iterator p = _params.begin();
    while(p != _params.end() && keyLength > p->first)
    {
        ++p;
    }
    _params.insert(p, KeyParamPair(keyLength, dh));
    return true;
}

DH*
IceSSL::DHParams::get(int keyLength)
{
    //
    // First check the set of parameters specified by the user.
    // Return the first set whose key length is at least keyLength.
    //
    for(ParamList::iterator p = _params.begin(); p != _params.end(); ++p)
    {
        if(p->first >= keyLength)
        {
            return p->second;
        }
    }

    //
    // No match found. Use one of the predefined parameter sets instead.
    //
    IceUtil::Mutex::Lock sync(*this);

    if(keyLength >= 4096)
    {
        if(!_dh4096)
        {
            _dh4096 = convertDH(dh4096_p, (int) sizeof(dh4096_p), dh4096_g, (int) sizeof(dh4096_g));
        }
        return _dh4096;
    }
    else if(keyLength >= 2048)
    {
        if(!_dh2048)
        {
            _dh2048 = convertDH(dh2048_p, (int) sizeof(dh2048_p), dh2048_g, (int) sizeof(dh2048_g));
        }
        return _dh2048;
    }
    else if(keyLength >= 1024)
    {
        if(!_dh1024)
        {
            _dh1024 = convertDH(dh1024_p, (int) sizeof(dh1024_p), dh1024_g, (int) sizeof(dh1024_g));
        }
        return _dh1024;
    }
    else
    {
        if(!_dh512)
        {
            _dh512 = convertDH(dh512_p, (int) sizeof(dh512_p), dh512_g, (int) sizeof(dh512_g));
        }
        return _dh512;
    }
}

#   endif

string
IceSSL::getSslErrors(bool verbose)
{
    ostringstream ostr;

    const char* file;
    const char* data;
    int line;
    int flags;
    unsigned long err;
    int count = 0;
    while((err = ERR_get_error_line_data(&file, &line, &data, &flags)) != 0)
    {
        if(count > 0)
        {
            ostr << endl;
        }

        if(verbose)
        {
            if(count > 0)
            {
                ostr << endl;
            }

            char buf[200];
            ERR_error_string_n(err, buf, sizeof(buf));

            ostr << "error # = " << err << endl;
            ostr << "message = " << buf << endl;
            ostr << "location = " << file << ", " << line;
            if(flags & ERR_TXT_STRING)
            {
                ostr << endl;
                ostr << "data = " << data;
            }
        }
        else
        {
            const char* reason = ERR_reason_error_string(err);
            ostr << (reason == NULL ? "unknown reason" : reason);
            if(flags & ERR_TXT_STRING)
            {
                ostr << ": " << data;
            }
        }

        ++count;
    }

    ERR_clear_error();

    return ostr.str();
}

#elif defined(ICE_USE_SECURE_TRANSPORT)

string
IceSSL::errorToString(CFErrorRef err)
{
    ostringstream os;
    if(err)
    {
        CFStringRef s = CFErrorCopyDescription(err);
        os << "(error: " << CFErrorGetCode(err) << " description: " << fromCFString(s) << ")";
        CFRelease(s);
    }
    return os.str();
}

string
IceSSL::errorToString(OSStatus status)
{
    ostringstream os;
    os << "(error: " << status;
    CFStringRef s = SecCopyErrorMessageString(status, 0);
    if(s)
    {
        os << " description: " << fromCFString(s);
        CFRelease(s);
    }
    os << ")";
    return os.str();
}

std::string 
IceSSL::fromCFString(CFStringRef v)
{
    string s;
    if(v)
    {
        CFIndex size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(v), kCFStringEncodingUTF8);
        vector<char> buffer;
        buffer.resize(size + 1);
        CFStringGetCString(v, &buffer[0], buffer.size(), kCFStringEncodingUTF8);
        s.assign(&buffer[0]);
    }
    return s;
}

CFDictionaryRef
IceSSL::getCertificateProperty(SecCertificateRef cert, CFTypeRef key)
{
    CFArrayRef keys = CFArrayCreate(NULL, &key , 1, &kCFTypeArrayCallBacks);
    CFErrorRef err = 0;
    CFDictionaryRef values = SecCertificateCopyValues(cert, keys, &err);
    CFRelease(keys);
    
    if(err)
    {
        CertificateEncodingException ex(__FILE__, __LINE__, err);
        throw ex;
    }
    
    assert(values);
    CFDictionaryRef property = (CFDictionaryRef)CFDictionaryGetValue(values, key);
    if(property)
    {
        CFRetain(property);
    }
    CFRelease(values);
    return property;
}

namespace
{

//
// Retrive the certificate subject key identifier, the caller must release the returned CFData 
// object.
//
CFDataRef
getSubjectKeyIdentifier(SecCertificateRef cert)
{
    CFDataRef data = 0;
    CFDictionaryRef property = getCertificateProperty(cert, kSecOIDSubjectKeyIdentifier);
    if(property)
    {
        CFArrayRef propertyValues = (CFArrayRef)CFDictionaryGetValue(property, kSecPropertyKeyValue);
        for(int i = 0, length = CFArrayGetCount(propertyValues); i < length; ++i)
        {
            CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(propertyValues, i);
            CFStringRef label = (CFStringRef)CFDictionaryGetValue(dict, kSecPropertyKeyLabel);
            if(CFEqual(label, CFSTR("Key Identifier")))
            {
                data = (CFDataRef)CFDictionaryGetValue(dict, kSecPropertyKeyValue);
                CFRetain(data);
                break;
            }
        }
        CFRelease(property);
    }
    return data;
}

//
// Check the certificate basic constraints to check if the certificate is marked as a CA.
//
bool
isCA(SecCertificateRef cert)
{
    bool ca = false;
    CFDictionaryRef property = getCertificateProperty(cert, kSecOIDBasicConstraints);
    if(property)
    {
        CFArrayRef propertyValues = (CFArrayRef)CFDictionaryGetValue(property, kSecPropertyKeyValue);
        for(int i = 0, size = CFArrayGetCount(propertyValues); i < size; ++i)
        {
            CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(propertyValues, i);
            CFStringRef label = (CFStringRef)CFDictionaryGetValue(dict, kSecPropertyKeyLabel);
            if(CFEqual(label, CFSTR("Certificate Authority")))
            {
                CFStringRef value = (CFStringRef)CFDictionaryGetValue(dict, kSecPropertyKeyValue);
                if(CFEqual(value, CFSTR("Yes")))
                {
                    ca = true;
                }
                break;
            }
        }
        CFRelease(property);
    }
    return ca;
}

//
// Search the keychain for an existing item with the same hash and type,
// the hash is the certificate subject key identifier. For private key
// items the hash should match kSecAttrApplicationLabel attribute, for 
// certificate items it should match the kSecAttrSubjectKeyID attribute.
//
SecKeychainItemRef
copyMatching(SecKeychainRef keychain, CFDataRef hash, CFTypeRef type)
{
    assert(keychain);
    assert(hash);
    assert(type == kSecClassKey || type == kSecClassCertificate);
    
    const void* values[] = {keychain};
    CFArrayRef searchList = CFArrayCreate(kCFAllocatorDefault, values, 1, &kCFTypeArrayCallBacks);
    
    CFMutableDictionaryRef query =
        CFDictionaryCreateMutable(0, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFDictionarySetValue(query, kSecClass, type);
    CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitOne);
    CFDictionarySetValue(query, kSecMatchSearchList, searchList);
    CFDictionarySetValue(query, type == kSecClassKey ? kSecAttrApplicationLabel : kSecAttrSubjectKeyID, hash);
    CFDictionarySetValue(query, kSecReturnRef, kCFBooleanTrue);
    
    SecKeychainItemRef item = 0;
    OSStatus err = SecItemCopyMatching(query, (CFTypeRef*)&item);
    
    CFRelease(searchList);
    CFRelease(query);
    
    if(err != noErr && err != errSecItemNotFound)
    {
        throw CertificateReadException(__FILE__, __LINE__, 
                                       "Error searching for keychain items\n" + errorToString(err));
    }
    
    return item;
}

//
// Add an item to the keychain, if the keychain already has this item return the existing item,
// otherwise return the new added item.
//
SecKeychainItemRef
addToKeychain(SecKeychainRef keychain, SecKeychainItemRef item, CFDataRef hash, CFTypeRef type)
{
    assert(keychain);
    assert(item);
    assert(hash);
    
    SecKeychainItemRef newItem = copyMatching(keychain, hash, type);
    if(!newItem)
    {
        CFMutableDictionaryRef query = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                            0,
                                                            &kCFTypeDictionaryKeyCallBacks,
                                                            &kCFTypeDictionaryValueCallBacks);
        
        CFDictionarySetValue(query, kSecUseKeychain, keychain);
        CFDictionarySetValue(query, kSecClass, type);
        CFDictionarySetValue(query, kSecValueRef, item);
        CFDictionarySetValue(query, kSecReturnRef, kCFBooleanTrue);
        
        CFArrayRef added = 0;
        OSStatus err = SecItemAdd(query, (CFTypeRef*)&added);
        CFRelease(query);
        
        if(err != noErr)
        {
            ostringstream os;
            os << "Failure adding " << (type == kSecClassKey ? "key" : "certificate") 
               << " to keychain\n" << errorToString(err);
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }
        newItem = (SecKeychainItemRef)CFArrayGetValueAtIndex(added, 0);
        CFRetain(newItem);
        CFRelease(added);
    }
    
    assert(newItem);
    
    return newItem;
}

//
// Load keychain items (Certificates or Private Keys) from a file. On return items param contain
// the list of items, the caller must release it.
//
void
loadKeychainItems(CFArrayRef* items, CFTypeRef type, const string& file, SecExternalFormat* format, 
                  SecKeychainRef keychain, const string& passphrase, const PasswordPromptPtr& prompt,
                  int passwordRetryMax)
{
    assert(type == kSecClassCertificate || type == kSecClassKey);
    vector<char> buffer;
    readFile(file, buffer);
    
    CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, 
                                                 reinterpret_cast<const UInt8*>(&buffer[0]), 
                                                 buffer.size(),
                                                 kCFAllocatorNull);
    
    SecExternalItemType itemType = kSecItemTypeUnknown;
    
    SecItemImportExportKeyParameters params;
    memset(&params, 0, sizeof(params));
    params.version =  SEC_KEY_IMPORT_EXPORT_PARAMS_VERSION;
    
    {
        const void* values[] = {kSecACLAuthorizationAny};
        params.keyUsage = CFArrayCreate(0, values, 1, 0);
    }
    
    params.flags |= kSecKeyNoAccessControl;

    OSStatus err = noErr;
    int count = 0;
    
    while(true)
    {
        if(!passphrase.empty())
        {
            assert(!params.passphrase);
            params.passphrase = toCFString(passphrase);
        }
        err = SecItemImport(data, 0, format, &itemType, 0, &params, *format == kSecFormatPKCS12 ? keychain : 0, 
                            items);
        if(params.passphrase)
        {
            CFRelease(params.passphrase);
            params.passphrase = 0;
        }
        
        if(err == noErr)
        {
            break;
        }
        
        //
        // Try PKCS12 format.
        //
        if(err == errSecUnknownFormat && *format != kSecFormatPKCS12)
        {
            *format = kSecFormatPKCS12;
            itemType = kSecItemTypeAggregate;
            continue;
        }
        
        //
        // Error
        //
        if(!passphrase.empty() || (err != errSecPassphraseRequired && 
                                   err != errSecInvalidData &&
                                   err != errSecPkcs12VerifyFailure))
        {
            break;
        }
        
        if(prompt && count < passwordRetryMax)
        {
            params.passphrase = toCFString(prompt->getPassword());
        }
        //
        // Configure the default OS X password prompt if passphrase is required
        // and the user doesn't provide a passphrase or password prompt.
        //
        else if(!prompt && !(params.flags & kSecKeySecurePassphrase))
        {
            params.flags |= kSecKeySecurePassphrase;
            ostringstream os;
            os << "Enter the password for\n" << file;
            params.alertPrompt = toCFString(os.str());
            continue;
        }
        //
        // Password retry.
        //
        if(++count >= passwordRetryMax)
        {
            break;
        }
    }
    
    if(params.alertPrompt)
    {
        CFRelease(params.alertPrompt);
    }
    
    CFRelease(data);
    
    if(err != noErr)
    {
        ostringstream os;
        os << "Error reading " << (type == kSecClassCertificate ? "certificate " : "private key ")
           << "from file: `" << file << "'\n" << errorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }
}

}

//
// Helper function to generate the private key label (display name) used
// in the keychain.
//
string
IceSSL::keyLabel(SecCertificateRef cert)
{
    CFStringRef commonName;
    OSStatus err = SecCertificateCopyCommonName(cert, &commonName);
    if(err != noErr)
    {
        throw CertificateReadException(__FILE__, __LINE__, "certificate error:\n" + errorToString(err));
    }
    string label = fromCFString(commonName);
    CFRelease(commonName);
    return label.empty() ? "Imported Private Key" : (label + " - Private Key");
}

//
// Imports a certificate private key and optionally add it to a keychain.
//
void
IceSSL::loadPrivateKey(SecKeyRef* key, const string& label, CFDataRef hash, SecKeychainRef keychain,
                       const string& file, const string& passphrase, const PasswordPromptPtr& prompt,
                       int passwordRetryMax)
{
    assert(key);
    CFArrayRef items = 0;
    try
    {
        SecExternalFormat format = kSecFormatUnknown;
        loadKeychainItems(&items, kSecClassKey, file, &format, keychain, passphrase, prompt, passwordRetryMax);
        if(items)
        {
            int count = CFArrayGetCount(items);
            for(int i = 0; i < count; ++i)
            {
                SecKeychainItemRef item = (SecKeychainItemRef)CFArrayGetValueAtIndex(items, 0);
                if(SecKeyGetTypeID() == CFGetTypeID(item))
                {
                    CFRetain(item);
                    *key = (SecKeyRef)item;
                    
                    CFRelease(items);
                    items = 0;
                    
                    if(keychain)
                    {
                        SecKeychainItemRef newItem = addToKeychain(keychain, item, hash, kSecClassKey);
                        assert(newItem);
                        CFRelease(*key);
                        *key = (SecKeyRef)newItem;
                        if(hash)
                        {
                            //
                            // Create the association between the private  key and the certificate, 
                            // kSecKeyLabel attribute should match the subject key identifier.
                            //
                            SecKeychainAttribute attr;
                            attr.tag = kSecKeyLabel;
                            attr.data = (void*)CFDataGetBytePtr(hash);
                            attr.length = CFDataGetLength(hash);
                            
                            SecKeychainAttributeList attrs;
                            attrs.attr = &attr;
                            attrs.count = 1;
                            
                            SecKeychainItemModifyAttributesAndData(newItem, &attrs, 0, 0);
                        }
                        
                        if(!label.empty())
                        {
                            //
                            // kSecKeyPrintName attribute correspond to the keychain display
                            // name.
                            //
                            SecKeychainAttribute att;
                            att.tag = kSecKeyPrintName;
                            att.data = (void*)label.c_str();
                            att.length = label.size();
                            
                            SecKeychainAttributeList attrs;
                            attrs.attr = &att;
                            attrs.count = 1;
                            
                            SecKeychainItemModifyAttributesAndData(newItem, &attrs, 0, 0);
                        }
                    }
                    break;
                }
            }
        }
        
        if(!*key)
        {
            throw CertificateReadException(__FILE__, __LINE__, 
                                            "Certificate error:\n error importing certificate from " + file);
        }
    }
    catch(...)
    {        
        if(items)
        {
            CFRelease(items);
        }
        
        if(*key)
        {
            CFRelease(*key);
            *key = 0;
        }
        
        throw;
    }
}

//
// Imports a certificate and private key and optionally add then to a keychain.
//
void
IceSSL::loadCertificate(SecCertificateRef* cert, CFDataRef* hash, SecKeyRef* key, SecKeychainRef keychain, 
                        const string& file, const string& passphrase, const PasswordPromptPtr& prompt,
                        int passwordRetryMax)
{
    assert(cert);
    CFArrayRef items = 0;
    SecIdentityRef identity = 0;
    
    try
    {
        SecExternalFormat format = kSecFormatUnknown;
        loadKeychainItems(&items, kSecClassCertificate, file, &format, keychain, passphrase, prompt, passwordRetryMax);
        
        if(items)
        {
            int count = CFArrayGetCount(items);
            
            for(int i = 0; i < count; ++i)
            {
                SecKeychainItemRef item = (SecKeychainItemRef)CFArrayGetValueAtIndex(items, i);
                if(format == kSecFormatPKCS12)
                {
                    OSStatus err = noErr;
                    if(SecIdentityGetTypeID() == CFGetTypeID(item))
                    {
                        if((err = SecIdentityCopyCertificate((SecIdentityRef)item, cert)) != noErr)
                        {
                            throw CertificateReadException(__FILE__, __LINE__, "Certificate error:\n" + 
                                                        errorToString(err));
                        }
                        
                        if((err = SecIdentityCopyPrivateKey((SecIdentityRef)item, key)) != noErr)
                        {
                            throw CertificateReadException(__FILE__, __LINE__, "Certificate error:\n" + 
                                                        errorToString(err));
                        }
                        break;
                    }
                    else if(SecCertificateGetTypeID() == CFGetTypeID(item))
                    {
                        CFRetain(item);
                        *cert = (SecCertificateRef)item;
                        
                        if((err = SecIdentityCreateWithCertificate(keychain, *cert, &identity)) != noErr)
                        {
                            throw CertificateReadException(__FILE__, __LINE__, "Certificate error:\n" + 
                                                        errorToString(err));
                        }
                        if((err = SecIdentityCopyPrivateKey(identity, key)) != noErr)
                        {
                            throw CertificateReadException(__FILE__, __LINE__, "Certificate error:\n" + 
                                                        errorToString(err));
                        }
                        
                        CFRelease(identity);
                        identity = 0;
                        break;
                    }
                }
                else if(SecCertificateGetTypeID() == CFGetTypeID(item))
                {
                    CFRetain(item);
                    *cert = (SecCertificateRef)item;
                    
                    //
                    // Copy the public key hash, that is used when added the private key
                    // to create an association between the certificate and the corresponding
                    // private key.
                    //
                    if(hash)
                    {
                        *hash = getSubjectKeyIdentifier(*cert);
                        
                        if(keychain)
                        {
                            SecKeychainItemRef newItem = addToKeychain(keychain, item, *hash, kSecClassCertificate);
                            assert(newItem);
                            CFRelease(*cert);
                            *cert = (SecCertificateRef)newItem;
                        }
                    }
                    break;
                }
            }
            
            CFRelease(items);
            items = 0;
        }
        
        if(!*cert)
        {
            throw CertificateReadException(__FILE__, __LINE__, 
                                            "Certificate error:\n error importing certificate from " + file);
        }
    }
    catch(...)
    {
        if(*cert)
        {
            CFRelease(*cert);
            *cert = 0;
        }
        
        if(hash && *hash)
        {
            CFRelease(*hash);
            *hash = 0;
        }
        
        if(items)
        {
            CFRelease(items);
        }
        
        if(identity)
        {
            CFRelease(identity);
        }
        
        if(key && *key)
        {
            CFRelease(*key);
            *key = 0;
        }
        
        throw;
    }
}

CFArrayRef
IceSSL::loadCACertificates(const string& file, const string& passphrase, const PasswordPromptPtr& prompt,
                           int passwordRetryMax)
{
    CFArrayRef items = 0;
    SecExternalFormat format = kSecFormatUnknown;
    loadKeychainItems(&items, kSecClassCertificate, file, &format, 0, passphrase, prompt, passwordRetryMax);
    CFMutableArrayRef certificateAuthorities = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if(items)
    {
        for(CFIndex i = 0, size = CFArrayGetCount(items); i < size; ++i)
        {
            SecCertificateRef cert = (SecCertificateRef)CFArrayGetValueAtIndex(items, i);
            if(isCA(cert))
            {
                CFArrayAppendValue(certificateAuthorities, cert);
            }
        }
        CFRelease(items);
    }
    return certificateAuthorities;
}

SecCertificateRef
IceSSL::findCertificates(SecKeychainRef keychain, const string& prop, const string& value)
{
    //
    //  Search the keychain using key:value pairs. The following keys are supported:
    //
    //   Label
    //   Serial
    //   Subject
    //   SubjectKeyId
    //
    //   A value must be enclosed in single or double quotes if it contains whitespace.
    //
    CFMutableDictionaryRef query =
        CFDictionaryCreateMutable(0, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    const void* values[] = { keychain };
    CFArrayRef searchList = CFArrayCreate(kCFAllocatorDefault, values, 1, &kCFTypeArrayCallBacks);
    
    CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitOne);
    CFDictionarySetValue(query, kSecMatchSearchList, searchList);
    CFDictionarySetValue(query, kSecClass, kSecClassCertificate);
    CFDictionarySetValue(query, kSecReturnRef, kCFBooleanTrue);
    CFDictionarySetValue(query, kSecMatchCaseInsensitive, kCFBooleanTrue);

    size_t start = 0;
    size_t pos;
    while((pos = value.find(':', start)) != string::npos)
    {
        string field = IceUtilInternal::toUpper(IceUtilInternal::trim(value.substr(start, pos - start)));
        string arg;
        try
        {
            if(field != "LABEL" && field != "SERIAL" && field != "SUBJECT" && field != "SUBJECTKEYID")
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unknown key in `" + value + "'");
            }

            start = pos + 1;
            while(start < value.size() && (value[start] == ' ' || value[start] == '\t'))
            {
                ++start;
            }
            
            if(start == value.size())
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: missing argument in `" + value + "'");
            }

            if(value[start] == '"' || value[start] == '\'')
            {
                size_t end = start;
                ++end;
                while(end < value.size())
                {
                    if(value[end] == value[start] && value[end - 1] != '\\')
                    {
                        break;
                    }
                    ++end;
                }
                if(end == value.size() || value[end] != value[start])
                {
                    throw PluginInitializationException(__FILE__, __LINE__, 
                                                        "IceSSL: unmatched quote in `" + value + "'");
                }
                ++start;
                arg = value.substr(start, end - start);
                start = end + 1;
            }
            else
            {
                size_t end = value.find_first_of(" \t", start);
                if(end == string::npos)
                {
                    arg = value.substr(start);
                    start = value.size();
                }
                else
                {
                    arg = value.substr(start, end - start);
                    start = end + 1;
                }
            }
        }
        catch(...)
        {
            CFRelease(searchList);
            CFRelease(query);
            throw;
        }

        if(field == "SUBJECT" || field == "LABEL")
        {
            CFDictionarySetValue(query, field == "LABEL" ? kSecAttrLabel : kSecMatchSubjectContains, toCFString(arg));
        }
        else if(field == "SUBJECTKEYID" || field == "SERIAL")
        {
            vector<unsigned char> buffer;
            if(!parseBytes(arg, buffer))
            {
                throw PluginInitializationException(__FILE__, __LINE__, 
                                            "IceSSL: invalid value `" + value + "' for property `" + prop + "'");
            }
            CFDataRef data = CFDataCreate(kCFAllocatorDefault, &buffer[0], buffer.size());
            CFDictionarySetValue(query, field == "SUBJECTKEYID" ? kSecAttrSubjectKeyID : kSecAttrSerialNumber, data);
        }
    }

    SecKeychainItemRef item = 0;
    OSStatus err = SecItemCopyMatching(query, (CFTypeRef*)&item);
    CFRelease(searchList);
    CFRelease(query);
    if(err != noErr && err != errSecItemNotFound)
    {
        throw PluginInitializationException(__FILE__, __LINE__, 
                                            "Error searching for keychain items\n" + errorToString(err));
    }
    return (SecCertificateRef)item;
}
#elif defined(ICE_USE_SCHANNEL)

namespace
{
//
// Parse a string of the form "location.name" into two parts.
//
void
parseStore(const string& prop, const string& store, DWORD& loc, string& sname)
{
    size_t pos = store.find('.');
    if(pos == string::npos)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: property `" + prop + "' has invalid format");
    }

    const string sloc = IceUtilInternal::toUpper(store.substr(0, pos));
    if(sloc == "CURRENTUSER")
    {
        loc = CERT_SYSTEM_STORE_CURRENT_USER;
    }
    else if(sloc == "LOCALMACHINE")
    {
        loc = CERT_SYSTEM_STORE_LOCAL_MACHINE;
    }
    else
    {
        throw PluginInitializationException(__FILE__, __LINE__, 
                                            "IceSSL: unknown store location `" + sloc + "' in " + prop);
    }

    sname = store.substr(pos + 1);
    if(sname.empty())
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: invalid store name in " + prop);
    }
}

void
addMatchingCertificates(HCERTSTORE source, HCERTSTORE target, DWORD findType, const void* findParam)
{
    PCCERT_CONTEXT next = 0;
    do
    { 
        if((next = CertFindCertificateInStore(source, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, 
                                              findType, findParam, next)))
        {
            if(!CertAddCertificateContextToStore(target, next, CERT_STORE_ADD_ALWAYS, 0))
            {
                throw PluginInitializationException(__FILE__, __LINE__, 
                    "IceSSL: error adding certificate to store:\n" + IceUtilInternal::lastErrorToString());
            }
        }
    }
    while(next);
}

}

vector<PCCERT_CONTEXT> 
IceSSL::findCertificates(const string& prop, const string& storeSpec, const string& value, vector<HCERTSTORE>& stores)
{
    DWORD storeLoc = 0;
    string storeName;
    parseStore(prop, storeSpec, storeLoc, storeName);

    HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, storeLoc, stringToWstring(storeName).c_str());
    if(!store)
    {
        throw PluginInitializationException(__FILE__, __LINE__, 
            "IceSSL: failure while opening store specified by " + prop + ":\n" + IceUtilInternal::lastErrorToString());
    }

    //
    // Start with all of the certificates in the collection and filter as necessary.
    //
    // - If the value is "*", return all certificates.
    // - Otherwise, search using key:value pairs. The following keys are supported:
    //
    //   Issuer
    //   IssuerDN
    //   Serial
    //   Subject
    //   SubjectDN
    //   SubjectKeyId
    //   Thumbprint
    //
    //   A value must be enclosed in single or double quotes if it contains whitespace.
    //
    HCERTSTORE tmpStore = 0;
    try
    {
        if(value != "*")
        {
            size_t start = 0;
            size_t pos;
            while((pos = value.find(':', start)) != string::npos)
            {
                string field = IceUtilInternal::toUpper(IceUtilInternal::trim(value.substr(start, pos - start)));
                if(field != "SUBJECT" && field != "SUBJECTDN" && field != "ISSUER" && field != "ISSUERDN" && 
                   field != "THUMBPRINT" && field != "SUBJECTKEYID" && field != "SERIAL")
                {
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unknown key in `" + value + "'");
                }

                start = pos + 1;
                while(start < value.size() && (value[start] == ' ' || value[start] == '\t'))
                {
                    ++start;
                }
                
                if(start == value.size())
                {
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: missing argument in `" + value + "'");
                }

                string arg;
                if(value[start] == '"' || value[start] == '\'')
                {
                    size_t end = start;
                    ++end;
                    while(end < value.size())
                    {
                        if(value[end] == value[start] && value[end - 1] != '\\')
                        {
                            break;
                        }
                        ++end;
                    }
                    if(end == value.size() || value[end] != value[start])
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                            "IceSSL: unmatched quote in `" + value + "'");
                    }
                    ++start;
                    arg = value.substr(start, end - start);
                    start = end + 1;
                }
                else
                {
                    size_t end = value.find_first_of(" \t", start);
                    if(end == string::npos)
                    {
                        arg = value.substr(start);
                        start = value.size();
                    }
                    else
                    {
                        arg = value.substr(start, end - start);
                        start = end + 1;
                    }
                }

                tmpStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, 0);
                if(!tmpStore)
                {
                    throw PluginInitializationException(__FILE__, __LINE__, 
                                "IceSSL: error adding certificate to store:\n" + IceUtilInternal::lastErrorToString());
                }

                if(field == "SUBJECT" || field == "ISSUER")
                {
                    const wstring argW = stringToWstring(arg);
                    DWORD findType = field == "SUBJECT" ? CERT_FIND_SUBJECT_STR : CERT_FIND_ISSUER_STR;
                    addMatchingCertificates(store, tmpStore, findType, argW.c_str());
                }
                else if(field == "SUBJECTDN" || field == "ISSUERDN")
                {
                    const wstring argW = stringToWstring(arg);
                    DWORD length = 0;
                    if(!CertStrToNameW(X509_ASN_ENCODING, argW.c_str(), CERT_OID_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                                       0, 0, &length, 0))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                "IceSSL: invalid value `" + value + "' for property `" + prop + "'\n" +
                                                IceUtilInternal::lastErrorToString());
                    }

                    vector<BYTE> buffer(length);
                    if(!CertStrToNameW(X509_ASN_ENCODING, argW.c_str(), CERT_OID_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                                       0, &buffer[0], &length, 0))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                "IceSSL: invalid value `" + value + "' for property `" + prop + "'\n" +
                                                IceUtilInternal::lastErrorToString());
                    }

                    CERT_NAME_BLOB name = { length, &buffer[0] };
                    DWORD findType = field == "SUBJECTDN" ? CERT_FIND_SUBJECT_NAME : CERT_FIND_ISSUER_NAME;
                    addMatchingCertificates(store, tmpStore, findType, &name);
                }
                else if(field == "THUMBPRINT" || field == "SUBJECTKEYID")
                {
                    vector<BYTE> buffer;
                    if(!parseBytes(arg, buffer))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                "IceSSL: invalid value `" + value + "' for property `" + prop + "'");
                    }

                    CRYPT_HASH_BLOB hash = { static_cast<DWORD>(buffer.size()), &buffer[0] };
                    DWORD findType = field == "THUMBPRINT" ? CERT_FIND_HASH : CERT_FIND_KEY_IDENTIFIER;
                    addMatchingCertificates(store, tmpStore, findType, &hash);
                }
                else if(field == "SERIAL")
                {
                    vector<BYTE> buffer;
                    if(!parseBytes(arg, buffer))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                "IceSSL: invalid value `" + value + "' for property `" + prop + "'");
                    }
                    
                    CRYPT_INTEGER_BLOB serial = { static_cast<DWORD>(buffer.size()), &buffer[0] };
                    PCCERT_CONTEXT next = 0;
                    do
                    {
                        if((next = CertFindCertificateInStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, 
                                                              CERT_FIND_ANY, 0, next)))
                        {
                            if(CertCompareIntegerBlob(&serial, &next->pCertInfo->SerialNumber))
                            {
                                if(!CertAddCertificateContextToStore(tmpStore, next, CERT_STORE_ADD_ALWAYS, 0))
                                {
                                    throw PluginInitializationException(__FILE__, __LINE__, 
                                                                    "IceSSL: error adding certificate to store:\n" +
                                                                    IceUtilInternal::lastErrorToString());
                                }
                            }
                        }
                    }
                    while(next);
                }
                CertCloseStore(store, 0);
                store = tmpStore;
            }
        }
    }
    catch(...)
    {
        if(store && store != tmpStore)
        {
            CertCloseStore(store, 0);
        }

        if(tmpStore)
        {
            CertCloseStore(tmpStore, 0);
            tmpStore = 0;
        }
        throw;
    }

    vector<PCCERT_CONTEXT> certs;
    if(store)
    {
        PCCERT_CONTEXT next = 0;
        do
        { 
            if((next = CertFindCertificateInStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, 0, 
                                                  next)))
            {
                certs.push_back(next);
            }
        }
        while(next);
        stores.push_back(store);
    }
    return certs;
}
#endif

bool
IceSSL::checkPath(string& path, const string& defaultDir, bool dir)
{
    //
    // Check if file exists. If not, try prepending the default
    // directory and check again. If the path exists, the string
    // argument is modified and true is returned. Otherwise
    // false is returned.
    //
    IceUtilInternal::structstat st;
    int err = IceUtilInternal::stat(path, &st);
    if(err == 0)
    {
        return dir ? S_ISDIR(st.st_mode) != 0 : S_ISREG(st.st_mode) != 0;
    }

    if(!defaultDir.empty())
    {
        string s = defaultDir + IceUtilInternal::separator + path;
        err = ::IceUtilInternal::stat(s.c_str(), &st);
        if(err == 0 && ((!dir && S_ISREG(st.st_mode)) || (dir && S_ISDIR(st.st_mode))))
        {
            path = s;
            return true;
        }
    }

    return false;
}
