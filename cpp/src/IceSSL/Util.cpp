// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
//
// Avoid old style cast warnings from OpenSSL macros
//
#  pragma GCC diagnostic ignored "-Wold-style-cast"
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

    if(!buffer.empty())
    {
        is.read(&buffer[0], buffer.size());
        if(!is.good())
        {
            throw CertificateReadException(__FILE__, __LINE__, "error reading file " + file);
        }
    }
}

#ifndef ICE_USE_OPENSSL

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

#endif

#if defined(ICE_USE_OPENSSL)

namespace
{

#  ifndef OPENSSL_NO_DH
#    if OPENSSL_VERSION_NUMBER < 0x10100000L

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

#    else
//
// With OpenSSL 1.1.0 is no longer possible to acess the DH p and g
// data members to set the DH params. We still use the same default
// parameters but they were converted to DER format using 
// i2d_DHparams and can be restored using d2i_DHparams

unsigned char dh512[] =
{
    0x30,0x46,0x02,0x41,0x00,0xF5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,
    0x40,0x18,0x11,0x8D,0x7C,0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,
    0x03,0x19,0xC8,0x07,0x29,0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,
    0xAB,0xD0,0x0A,0x50,0x9B,0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,
    0x5D,0x41,0x9F,0x9C,0x7C,0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,
    0xAB,0xA2,0x5E,0xC3,0x55,0xE9,0x2A,0x05,0x5F,0x02,0x01,0x02,
};

unsigned char dh1024[] =
{
    0x30,0x81,0x87,0x02,0x81,0x81,0x00,0xF4,0x88,0xFD,0x58,0x4E,
    0x49,0xDB,0xCD,0x20,0xB4,0x9D,0xE4,0x91,0x07,0x36,0x6B,0x33,
    0x6C,0x38,0x0D,0x45,0x1D,0x0F,0x7C,0x88,0xB3,0x1C,0x7C,0x5B,
    0x2D,0x8E,0xF6,0xF3,0xC9,0x23,0xC0,0x43,0xF0,0xA5,0x5B,0x18,
    0x8D,0x8E,0xBB,0x55,0x8C,0xB8,0x5D,0x38,0xD3,0x34,0xFD,0x7C,
    0x17,0x57,0x43,0xA3,0x1D,0x18,0x6C,0xDE,0x33,0x21,0x2C,0xB5,
    0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,0x18,0x11,0x8D,0x7C,0x84,
    0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,0x19,0xC8,0x07,0x29,0x7A,
    0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,0xD0,0x0A,0x50,0x9B,0x02,
    0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,0x41,0x9F,0x9C,0x7C,0xBD,
    0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,0xA2,0x5E,0xC3,0x55,0xE9,
    0x2F,0x78,0xC7,0x02,0x01,0x02,
};

unsigned char dh2048[] =
{
    0x30,0x82,0x01,0x08,0x02,0x82,0x01,0x01,0x00,0xF6,0x42,0x57,
    0xB7,0x08,0x7F,0x08,0x17,0x72,0xA2,0xBA,0xD6,0xA9,0x42,0xF3,
    0x05,0xE8,0xF9,0x53,0x11,0x39,0x4F,0xB6,0xF1,0x6E,0xB9,0x4B,
    0x38,0x20,0xDA,0x01,0xA7,0x56,0xA3,0x14,0xE9,0x8F,0x40,0x55,
    0xF3,0xD0,0x07,0xC6,0xCB,0x43,0xA9,0x94,0xAD,0xF7,0x4C,0x64,
    0x86,0x49,0xF8,0x0C,0x83,0xBD,0x65,0xE9,0x17,0xD4,0xA1,0xD3,
    0x50,0xF8,0xF5,0x59,0x5F,0xDC,0x76,0x52,0x4F,0x3D,0x3D,0x8D,
    0xDB,0xCE,0x99,0xE1,0x57,0x92,0x59,0xCD,0xFD,0xB8,0xAE,0x74,
    0x4F,0xC5,0xFC,0x76,0xBC,0x83,0xC5,0x47,0x30,0x61,0xCE,0x7C,
    0xC9,0x66,0xFF,0x15,0xF9,0xBB,0xFD,0x91,0x5E,0xC7,0x01,0xAA,
    0xD3,0x5B,0x9E,0x8D,0xA0,0xA5,0x72,0x3A,0xD4,0x1A,0xF0,0xBF,
    0x46,0x00,0x58,0x2B,0xE5,0xF4,0x88,0xFD,0x58,0x4E,0x49,0xDB,
    0xCD,0x20,0xB4,0x9D,0xE4,0x91,0x07,0x36,0x6B,0x33,0x6C,0x38,
    0x0D,0x45,0x1D,0x0F,0x7C,0x88,0xB3,0x1C,0x7C,0x5B,0x2D,0x8E,
    0xF6,0xF3,0xC9,0x23,0xC0,0x43,0xF0,0xA5,0x5B,0x18,0x8D,0x8E,
    0xBB,0x55,0x8C,0xB8,0x5D,0x38,0xD3,0x34,0xFD,0x7C,0x17,0x57,
    0x43,0xA3,0x1D,0x18,0x6C,0xDE,0x33,0x21,0x2C,0xB5,0x2A,0xFF,
    0x3C,0xE1,0xB1,0x29,0x40,0x18,0x11,0x8D,0x7C,0x84,0xA7,0x0A,
    0x72,0xD6,0x86,0xC4,0x03,0x19,0xC8,0x07,0x29,0x7A,0xCA,0x95,
    0x0C,0xD9,0x96,0x9F,0xAB,0xD0,0x0A,0x50,0x9B,0x02,0x46,0xD3,
    0x08,0x3D,0x66,0xA4,0x5D,0x41,0x9F,0x9C,0x7C,0xBD,0x89,0x4B,
    0x22,0x19,0x26,0xBA,0xAB,0xA2,0x5E,0xC3,0x55,0xE9,0x32,0x0B,
    0x3B,0x02,0x01,0x02,
};

unsigned char dh4096[] =
{
    0x30,0x82,0x02,0x08,0x02,0x82,0x02,0x01,0x00,0xFA,0x14,0x72,
    0x52,0xC1,0x4D,0xE1,0x5A,0x49,0xD4,0xEF,0x09,0x2D,0xC0,0xA8,
    0xFD,0x55,0xAB,0xD7,0xD9,0x37,0x04,0x28,0x09,0xE2,0xE9,0x3E,
    0x77,0xE2,0xA1,0x7A,0x18,0xDD,0x46,0xA3,0x43,0x37,0x23,0x90,
    0x97,0xF3,0x0E,0xC9,0x03,0x50,0x7D,0x65,0xCF,0x78,0x62,0xA6,
    0x3A,0x62,0x22,0x83,0xA1,0x2F,0xFE,0x79,0xBA,0x35,0xFF,0x59,
    0xD8,0x1D,0x61,0xDD,0x1E,0x21,0x13,0x17,0xFE,0xCD,0x38,0x87,
    0x9E,0xF5,0x4F,0x79,0x10,0x61,0x8D,0xD4,0x22,0xF3,0x5A,0xED,
    0x5D,0xEA,0x21,0xE9,0x33,0x6B,0x48,0x12,0x0A,0x20,0x77,0xD4,
    0x25,0x60,0x61,0xDE,0xF6,0xB4,0x4F,0x1C,0x63,0x40,0x8B,0x3A,
    0x21,0x93,0x8B,0x79,0x53,0x51,0x2C,0xCA,0xB3,0x7B,0x29,0x56,
    0xA8,0xC7,0xF8,0xF4,0x7B,0x08,0x5E,0xA6,0xDC,0xA2,0x45,0x12,
    0x56,0xDD,0x41,0x92,0xF2,0xDD,0x5B,0x8F,0x23,0xF0,0xF3,0xEF,
    0xE4,0x3B,0x0A,0x44,0xDD,0xED,0x96,0x84,0xF1,0xA8,0x32,0x46,
    0xA3,0xDB,0x4A,0xBE,0x3D,0x45,0xBA,0x4E,0xF8,0x03,0xE5,0xDD,
    0x6B,0x59,0x0D,0x84,0x1E,0xCA,0x16,0x5A,0x8C,0xC8,0xDF,0x7C,
    0x54,0x44,0xC4,0x27,0xA7,0x3B,0x2A,0x97,0xCE,0xA3,0x7D,0x26,
    0x9C,0xAD,0xF4,0xC2,0xAC,0x37,0x4B,0xC3,0xAD,0x68,0x84,0x7F,
    0x99,0xA6,0x17,0xEF,0x6B,0x46,0x3A,0x7A,0x36,0x7A,0x11,0x43,
    0x92,0xAD,0xE9,0x9C,0xFB,0x44,0x6C,0x3D,0x82,0x49,0xCC,0x5C,
    0x6A,0x52,0x42,0xF8,0x42,0xFB,0x44,0xF9,0x39,0x73,0xFB,0x60,
    0x79,0x3B,0xC2,0x9E,0x0B,0xDC,0xD4,0xA6,0x67,0xF7,0x66,0x3F,
    0xFC,0x42,0x3B,0x1B,0xDB,0x4F,0x66,0xDC,0xA5,0x8F,0x66,0xF9,
    0xEA,0xC1,0xED,0x31,0xFB,0x48,0xA1,0x82,0x7D,0xF8,0xE0,0xCC,
    0xB1,0xC7,0x03,0xE4,0xF8,0xB3,0xFE,0xB7,0xA3,0x13,0x73,0xA6,
    0x7B,0xC1,0x0E,0x39,0xC7,0x94,0x48,0x26,0x00,0x85,0x79,0xFC,
    0x6F,0x7A,0xAF,0xC5,0x52,0x35,0x75,0xD7,0x75,0xA4,0x40,0xFA,
    0x14,0x74,0x61,0x16,0xF2,0xEB,0x67,0x11,0x6F,0x04,0x43,0x3D,
    0x11,0x14,0x4C,0xA7,0x94,0x2A,0x39,0xA1,0xC9,0x90,0xCF,0x83,
    0xC6,0xFF,0x02,0x8F,0xA3,0x2A,0xAC,0x26,0xDF,0x0B,0x8B,0xBE,
    0x64,0x4A,0xF1,0xA1,0xDC,0xEE,0xBA,0xC8,0x03,0x82,0xF6,0x62,
    0x2C,0x5D,0xB6,0xBB,0x13,0x19,0x6E,0x86,0xC5,0x5B,0x2B,0x5E,
    0x3A,0xF3,0xB3,0x28,0x6B,0x70,0x71,0x3A,0x8E,0xFF,0x5C,0x15,
    0xE6,0x02,0xA4,0xCE,0xED,0x59,0x56,0xCC,0x15,0x51,0x07,0x79,
    0x1A,0x0F,0x25,0x26,0x27,0x30,0xA9,0x15,0xB2,0xC8,0xD4,0x5C,
    0xCC,0x30,0xE8,0x1B,0xD8,0xD5,0x0F,0x19,0xA8,0x80,0xA4,0xC7,
    0x01,0xAA,0x8B,0xBA,0x53,0xBB,0x47,0xC2,0x1F,0x6B,0x54,0xB0,
    0x17,0x60,0xED,0x79,0x21,0x95,0xB6,0x05,0x84,0x37,0xC8,0x03,
    0xA4,0xDD,0xD1,0x06,0x69,0x8F,0x4C,0x39,0xE0,0xC8,0x5D,0x83,
    0x1D,0xBE,0x6A,0x9A,0x99,0xF3,0x9F,0x0B,0x45,0x29,0xD4,0xCB,
    0x29,0x66,0xEE,0x1E,0x7E,0x3D,0xD7,0x13,0x4E,0xDB,0x90,0x90,
    0x58,0xCB,0x5E,0x9B,0xCD,0x2E,0x2B,0x0F,0xA9,0x4E,0x78,0xAC,
    0x05,0x11,0x7F,0xE3,0x9E,0x27,0xD4,0x99,0xE1,0xB9,0xBD,0x78,
    0xE1,0x84,0x41,0xA0,0xDF,0x02,0x01,0x02,
};
#    endif

}

//
// Convert a predefined parameter set into a DH value.
//
#    if OPENSSL_VERSION_NUMBER >= 0x10100000L
static DH*
convertDH(const unsigned char* buf, int len)
{
    return d2i_DHparams(0, &buf, len);
}
#    else
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
#    endif

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
#    if OPENSSL_VERSION_NUMBER >= 0x10100000L
    if(keyLength >= 4096)
    {
        if(!_dh4096)
        {
            _dh4096 = convertDH(dh4096, (int) sizeof(dh4096));
        }
        return _dh4096;
    }
    else if(keyLength >= 2048)
    {
        if(!_dh2048)
        {
            _dh2048 = convertDH(dh2048, (int) sizeof(dh2048));
        }
        return _dh2048;
    }
    else if(keyLength >= 1024)
    {
        if(!_dh1024)
        {
            _dh1024 = convertDH(dh1024, (int) sizeof(dh1024));
        }
        return _dh1024;
    }
    else
    {
        if(!_dh512)
        {
            _dh512 = convertDH(dh512, (int) sizeof(dh512));
        }
        return _dh512;
    }
#    else
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
#    endif
}

#  endif

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
        ostringstream os;
        os << "IceSSL: error getting property for certificate:\n" << errorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
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
// Check the certificate basic constraints to check if the certificate is marked as a CA.
//
bool
isCA(SecCertificateRef cert)
{
    UniqueRef<CFDictionaryRef> property(getCertificateProperty(cert, kSecOIDBasicConstraints));
    if(property)
    {
        CFArrayRef propertyValues = (CFArrayRef)CFDictionaryGetValue(property.get(), kSecPropertyKeyValue);
        for(int i = 0, size = CFArrayGetCount(propertyValues); i < size; ++i)
        {
            CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(propertyValues, i);
            CFStringRef label = (CFStringRef)CFDictionaryGetValue(dict, kSecPropertyKeyLabel);
            if(CFEqual(label, CFSTR("Certificate Authority")))
            {
                return CFEqual((CFStringRef)CFDictionaryGetValue(dict, kSecPropertyKeyValue), CFSTR("Yes"));
            }
        }
    }
    return false;
}

//
// Load keychain items (Certificates or Private Keys) from a file. On return items param contain
// the list of items, the caller must release it.
//
CFArrayRef
loadKeychainItems(const string& file, SecExternalItemType type, SecKeychainRef keychain, const string& passphrase,
                  const PasswordPromptPtr& prompt, int retryMax)
{
    vector<char> buffer;
    readFile(file, buffer);
    UniqueRef<CFDataRef> data(CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
                                                          reinterpret_cast<const UInt8*>(&buffer[0]),
                                                          buffer.size(),
                                                          kCFAllocatorNull));

    SecItemImportExportKeyParameters params;
    memset(&params, 0, sizeof(params));
    params.version =  SEC_KEY_IMPORT_EXPORT_PARAMS_VERSION;
    params.flags |= kSecKeyNoAccessControl;
    if(!passphrase.empty())
    {
        params.passphrase = toCFString(passphrase);
    }

    CFArrayRef items;
    SecExternalItemType importType = type;
    SecExternalFormat format = type == kSecItemTypeUnknown ? kSecFormatPKCS12 : kSecFormatUnknown;
    UniqueRef<CFStringRef> path(toCFString(file));
    OSStatus err = SecItemImport(data.get(), path.get(), &format, &importType, 0, &params, keychain, &items);

    //
    // If passphrase failure and no password was configured, we obtain
    // the password from the given prompt or configure the import to
    // prompt the user with an alert dialog.
    //
    if(passphrase.empty() &&
       (err == errSecPassphraseRequired || err == errSecInvalidData || err == errSecPkcs12VerifyFailure))
    {
        if(!prompt)
        {
            params.flags |= kSecKeySecurePassphrase;
            ostringstream os;
            os << "Enter the password for\n" << file;
            params.alertPrompt = toCFString(os.str());
        }

        int count = 0;
        while((err == errSecPassphraseRequired || err == errSecInvalidData || err == errSecPkcs12VerifyFailure) &&
              count < retryMax)
        {
            if(prompt)
            {
                if(params.passphrase)
                {
                    CFRelease(params.passphrase);
                }
                params.passphrase = toCFString(prompt->getPassword());
            }
            err = SecItemImport(data.get(), path.get(), &format, &importType, 0, &params, keychain, &items);
            ++count;
        }

        if(params.alertPrompt)
        {
            CFRelease(params.alertPrompt);
        }
    }

    if(params.passphrase)
    {
        CFRelease(params.passphrase);
    }

    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: error reading " << (type == kSecItemTypePrivateKey ? "private key" : "certificate");
        os << " `" << file << "':\n" << errorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }

    if(type != kSecItemTypeUnknown && importType != kSecItemTypeAggregate && importType != type)
    {
        CFRelease(items);
        ostringstream os;
        os << "IceSSL: error reading " << (type == kSecItemTypePrivateKey ? "private key" : "certificate");
        os << " `" << file << "' doesn't contain the expected item";
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }

    return items;
}

}

//
// Imports a certificate private key and optionally add it to a keychain.
//
SecIdentityRef
IceSSL::loadPrivateKey(const string& file, SecCertificateRef cert, SecKeychainRef keychain, const string& password,
                       const PasswordPromptPtr& prompt, int retryMax)
{
    //
    // Check if we already imported the certificate
    //
    UniqueRef<CFDataRef> hash;
    UniqueRef<CFDictionaryRef> subjectKeyProperty(getCertificateProperty(cert, kSecOIDSubjectKeyIdentifier));
    if(subjectKeyProperty)
    {
        CFArrayRef values = (CFArrayRef)CFDictionaryGetValue(subjectKeyProperty.get(), kSecPropertyKeyValue);
        for(int i = 0; i < CFArrayGetCount(values); ++i)
        {
            CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(values, i);
            if(CFEqual(CFDictionaryGetValue(dict, kSecPropertyKeyLabel), CFSTR("Key Identifier")))
            {
                hash.retain(CFDictionaryGetValue(dict, kSecPropertyKeyValue));
                break;
            }
        }
    }

    const void* values[] = { keychain };
    UniqueRef<CFArrayRef> searchList(CFArrayCreate(kCFAllocatorDefault, values, 1, &kCFTypeArrayCallBacks));

    UniqueRef<CFMutableDictionaryRef> query(CFDictionaryCreateMutable(0,
                                                                      0,
                                                                      &kCFTypeDictionaryKeyCallBacks,
                                                                      &kCFTypeDictionaryValueCallBacks));

    CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
    CFDictionarySetValue(query.get(), kSecMatchLimit, kSecMatchLimitOne);
    CFDictionarySetValue(query.get(), kSecMatchSearchList, searchList.get());
    CFDictionarySetValue(query.get(), kSecAttrSubjectKeyID, hash.get());
    CFDictionarySetValue(query.get(), kSecReturnRef, kCFBooleanTrue);

    CFTypeRef value = 0;
    OSStatus err = SecItemCopyMatching(query.get(), &value);
    UniqueRef<SecCertificateRef> item(value);
    if(err == noErr)
    {
        //
        // If the certificate has already been imported, create the
        // identity. The key should also have been imported.
        //
        SecIdentityRef identity;
        err = SecIdentityCreateWithCertificate(keychain, item.get(), &identity);
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: error creating certificate identity:\n" << errorToString(err);
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }
        return identity;
    }
    else if(err != errSecItemNotFound)
    {
        ostringstream os;
        os << "IceSSL: error searching for keychain items:\n" << errorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }

    //
    // If the certificate isn't already in the keychain, load the
    // private key into the keychain and add the certificate.
    //
    UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypePrivateKey, keychain, password, prompt, retryMax));
    int count = CFArrayGetCount(items.get());
    UniqueRef<SecKeyRef> key;
    for(int i = 0; i < count; ++i)
    {
        SecKeychainItemRef item = (SecKeychainItemRef)CFArrayGetValueAtIndex(items.get(), 0);
        if(SecKeyGetTypeID() == CFGetTypeID(item))
        {
            key.retain(item);
            break;
        }
    }
    if(!key)
    {
        throw CertificateReadException(__FILE__, __LINE__, "IceSSL: no key in file `" + file + "'");
    }

    //
    // Add the certificate to the keychain
    //
    query.reset(CFDictionaryCreateMutable(kCFAllocatorDefault,
                                          0,
                                          &kCFTypeDictionaryKeyCallBacks,
                                          &kCFTypeDictionaryValueCallBacks));

    CFDictionarySetValue(query.get(), kSecUseKeychain, keychain);
    CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
    CFDictionarySetValue(query.get(), kSecValueRef, cert);
    CFDictionarySetValue(query.get(), kSecReturnRef, kCFBooleanTrue);

    value = 0;
    err = SecItemAdd(query.get(), (CFTypeRef*)&value);
    UniqueRef<CFArrayRef> added(value);
    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: failure adding certificate to keychain\n" << errorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }
    item.retain(CFArrayGetValueAtIndex(added.get(), 0));

    //
    // Create the association between the private  key and the certificate,
    // kSecKeyLabel attribute should match the subject key identifier.
    //
    vector<SecKeychainAttribute> attributes;
    if(hash)
    {
        SecKeychainAttribute attr;
        attr.tag = kSecKeyLabel;
        attr.data = (void*)CFDataGetBytePtr(hash.get());
        attr.length = CFDataGetLength(hash.get());
        attributes.push_back(attr);
    }

    //
    // kSecKeyPrintName attribute correspond to the keychain display
    // name.
    //
    string label;
    CFStringRef commonName = 0;
    if(SecCertificateCopyCommonName(item.get(), &commonName) == noErr)
    {
        label = fromCFString(commonName);
        CFRelease(commonName);

        SecKeychainAttribute attr;
        attr.tag = kSecKeyPrintName;
        attr.data = (void*)label.c_str();
        attr.length = label.size();
        attributes.push_back(attr);
    }

    SecKeychainAttributeList attrs;
    attrs.attr = &attributes[0];
    attrs.count = attributes.size();
    SecKeychainItemModifyAttributesAndData((SecKeychainItemRef)key.get(), &attrs, 0, 0);

    SecIdentityRef identity;
    err = SecIdentityCreateWithCertificate(keychain, item.get(), &identity);
    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: error creating certificate identity:\n" << errorToString(err);
        throw CertificateReadException(__FILE__, __LINE__, os.str());
    }
    return identity;
}

//
// Imports a certificate (it might contain an identity or certificate depending on the format).
//
CFArrayRef
IceSSL::loadCertificateChain(const string& file, const string& keyFile, SecKeychainRef keychain,
                             const string& password, const PasswordPromptPtr& prompt, int retryMax)
{
    if(keyFile.empty())
    {
        return loadKeychainItems(file, kSecItemTypeUnknown, keychain, password, prompt, retryMax);
    }
    else
    {
        //
        // Load the certificate, don't load into the keychain as it
        // might already have been imported.
        //
        UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypeCertificate, 0, password, prompt, retryMax));
        SecCertificateRef cert = (SecCertificateRef)CFArrayGetValueAtIndex(items.get(), 0);
        if(SecCertificateGetTypeID() != CFGetTypeID(cert))
        {
            ostringstream os;
            os << "IceSSL: couldn't find certificate in `" << file << "'";
            throw CertificateReadException(__FILE__, __LINE__, os.str());
        }

        //
        // Load the private key for the given certificate. This will
        // add the certificate/key to the keychain if they aren't
        // already present in the keychain.
        //
        UniqueRef<SecIdentityRef> identity(loadPrivateKey(keyFile, cert, keychain, password, prompt, retryMax));
        CFMutableArrayRef a = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, items.get());
        CFArraySetValueAtIndex(a, 0, identity.get());
        return a;
    }
}

SecCertificateRef
IceSSL::loadCertificate(const string& file)
{
    CFArrayRef items = loadKeychainItems(file, kSecItemTypeCertificate, 0, "", 0, 0);
    SecCertificateRef cert = (SecCertificateRef)CFArrayGetValueAtIndex(items, 0);
    CFRetain(cert);
    CFRelease(items);
    return cert;
}

CFArrayRef
IceSSL::loadCACertificates(const string& file)
{
    UniqueRef<CFArrayRef> items(loadKeychainItems(file, kSecItemTypeCertificate, 0, "", 0, 0));
    CFMutableArrayRef certificateAuthorities = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    int count = CFArrayGetCount(items.get());
    for(CFIndex i = 0; i < count; ++i)
    {
        SecCertificateRef cert = (SecCertificateRef)CFArrayGetValueAtIndex(items.get(), i);
        assert(SecCertificateGetTypeID() == CFGetTypeID(cert));
        if(isCA(cert))
        {
            CFArrayAppendValue(certificateAuthorities, cert);
        }
    }
    return certificateAuthorities;
}

SecCertificateRef
IceSSL::findCertificate(SecKeychainRef keychain, const string& value)
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
    UniqueRef<CFMutableDictionaryRef> query(CFDictionaryCreateMutable(0,
                                                                      0,
                                                                      &kCFTypeDictionaryKeyCallBacks,
                                                                      &kCFTypeDictionaryValueCallBacks));

    const void* values[] = { keychain };
    UniqueRef<CFArrayRef> searchList(CFArrayCreate(kCFAllocatorDefault, values, 1, &kCFTypeArrayCallBacks));

    CFDictionarySetValue(query.get(), kSecMatchLimit, kSecMatchLimitOne);
    CFDictionarySetValue(query.get(), kSecMatchSearchList, searchList.get());
    CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
    CFDictionarySetValue(query.get(), kSecReturnRef, kCFBooleanTrue);
    CFDictionarySetValue(query.get(), kSecMatchCaseInsensitive, kCFBooleanTrue);

    size_t start = 0;
    size_t pos;
    while((pos = value.find(':', start)) != string::npos)
    {
        string field = IceUtilInternal::toUpper(IceUtilInternal::trim(value.substr(start, pos - start)));
        string arg;
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
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unmatched quote in `" + value + "'");
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

        if(field == "SUBJECT" || field == "LABEL")
        {
            UniqueRef<CFStringRef> v(toCFString(arg));
            CFDictionarySetValue(query.get(), field == "LABEL" ? kSecAttrLabel : kSecMatchSubjectContains, v.get());
        }
        else if(field == "SUBJECTKEYID" || field == "SERIAL")
        {
            vector<unsigned char> buffer;
            if(!parseBytes(arg, buffer))
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: invalid value `" + value + "'");
            }
            UniqueRef<CFDataRef> v(CFDataCreate(kCFAllocatorDefault, &buffer[0], buffer.size()));
            CFDictionarySetValue(query.get(), field == "SUBJECTKEYID" ? kSecAttrSubjectKeyID : kSecAttrSerialNumber,
                                 v.get());
        }
    }

    if(CFDictionaryGetCount(query.get()) == 5)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: invalid value `" + value + "'");
    }

    SecCertificateRef cert = 0;
    OSStatus err = SecItemCopyMatching(query.get(), (CFTypeRef*)&cert);
    if(err != noErr)
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: find certificate `" + value + "' failed:\n" + errorToString(err));
    }
    return cert;
}

#elif defined(ICE_USE_SCHANNEL)

namespace
{

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
IceSSL::findCertificates(const string& location, const string& name, const string& value, vector<HCERTSTORE>& stores)
{
    DWORD storeLoc;
    if(location == "CurrentUser")
    {
        storeLoc = CERT_SYSTEM_STORE_CURRENT_USER;
    }
    else
    {
        storeLoc = CERT_SYSTEM_STORE_LOCAL_MACHINE;
    }

    HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, storeLoc, stringToWstring(name).c_str());
    if(!store)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: failed to open certificate store `" + name +
                                            "':\n" + IceUtilInternal::lastErrorToString());
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
            if(value.find(':', 0) == string::npos)
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: no key in `" + value + "'");
            }
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
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: missing argument in `" + value + "'");
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
                    DWORD flags[] = {
                        CERT_OID_NAME_STR,
                        CERT_OID_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                        CERT_OID_NAME_STR | CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG,
                        CERT_OID_NAME_STR | CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG | CERT_NAME_STR_REVERSE_FLAG
                    };
                    for(size_t i = 0; i < sizeof(flags) / sizeof(DWORD); ++i)
                    {
                        DWORD length = 0;
                        if(!CertStrToNameW(X509_ASN_ENCODING, argW.c_str(), flags[i], 0, 0, &length, 0))
                        {
                            throw PluginInitializationException(
                                __FILE__, __LINE__,
                                "IceSSL: invalid value `" + value + "' for `IceSSL.FindCert' property:\n" +
                                IceUtilInternal::lastErrorToString());
                        }

                        vector<BYTE> buffer(length);
                        if(!CertStrToNameW(X509_ASN_ENCODING, argW.c_str(), flags[i], 0, &buffer[0], &length, 0))
                        {
                            throw PluginInitializationException(
                                __FILE__, __LINE__,
                                "IceSSL: invalid value `" + value + "' for `IceSSL.FindCert' property:\n" +
                                IceUtilInternal::lastErrorToString());
                        }

                        CERT_NAME_BLOB name = { length, &buffer[0] };

                        DWORD findType = field == "SUBJECTDN" ? CERT_FIND_SUBJECT_NAME : CERT_FIND_ISSUER_NAME;
                        addMatchingCertificates(store, tmpStore, findType, &name);
                    }
                }
                else if(field == "THUMBPRINT" || field == "SUBJECTKEYID")
                {
                    vector<BYTE> buffer;
                    if(!parseBytes(arg, buffer))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: invalid `IceSSL.FindCert' property: can't decode the value");
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
                                                "IceSSL: invalid value `" + value + "' for `IceSSL.FindCert' property");
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
IceSSL::checkPath(const string& path, const string& defaultDir, bool dir, string& resolved)
{
    if(IceUtilInternal::isAbsolutePath(path))
    {
        if((dir && IceUtilInternal::directoryExists(path)) || (!dir && IceUtilInternal::fileExists(path)))
        {
            resolved = path;
            return true;
        }
        return false;
    }

    //
    // If a default directory is provided, the given path is relative to the default directory.
    //
    string tmp;
    if(!defaultDir.empty())
    {
        tmp = defaultDir + IceUtilInternal::separator + path;
    }
    else
    {
        tmp = path;
    }

    if((dir && IceUtilInternal::directoryExists(tmp)) || (!dir && IceUtilInternal::fileExists(tmp)))
    {
        resolved = tmp;
        return true;
    }
    return false;
}
