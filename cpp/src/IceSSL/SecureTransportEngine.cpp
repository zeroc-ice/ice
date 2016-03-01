// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/Config.h>

#include <IceUtil/FileUtil.h>
#include <IceUtil/StringUtil.h>

#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/Communicator.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>

#include <IceSSL/SecureTransportTransceiverI.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/SSLEngine.h>
#include <IceSSL/Util.h>

#ifdef ICE_USE_SECURE_TRANSPORT

#include <regex.h>

using namespace std;
using namespace IceUtil;
using namespace Ice;
using namespace IceSSL;

namespace
{

IceUtil::Mutex* staticMutex = 0;

class Init
{
public:

    Init()
    {
        staticMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete staticMutex;
        staticMutex = 0;
    }
};

Init init;

class RegExp : public IceUtil::Shared
{
public:

    RegExp(const string&);
    ~RegExp();
    bool match(const string&);

private:

    regex_t _preg;
};
typedef IceUtil::Handle<RegExp> RegExpPtr;

RegExp::RegExp(const string& regexp)
{
    int err = regcomp(&_preg, regexp.c_str(), REG_EXTENDED | REG_NOSUB);
    if(err)
    {
        throw IceUtil::SyscallException(__FILE__, __LINE__, err);
    }
}

RegExp::~RegExp()
{
    regfree(&_preg);
}

bool
RegExp::match(const string& value)
{
    return regexec(&_preg, value.c_str(), 0, 0, 0) == 0;
}

struct CipherExpression
{
    bool negation;
    string cipher;
    RegExpPtr re;
};

class CiphersHelper
{
public:

    static void initialize();
    static SSLCipherSuite cipherForName(const string& name);
    static string cipherName(SSLCipherSuite cipher);
    static map<string, SSLCipherSuite> ciphers();

private:

    static map<string, SSLCipherSuite> _ciphers;
};

map<string, SSLCipherSuite> CiphersHelper::_ciphers;

//
// Initialize a dictionary with the names of ciphers
//
void
CiphersHelper::initialize()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
    if(_ciphers.empty())
    {
        _ciphers["NULL_WITH_NULL_NULL"] = SSL_NULL_WITH_NULL_NULL;
        _ciphers["RSA_WITH_NULL_MD5"] = SSL_RSA_WITH_NULL_MD5;
        _ciphers["RSA_WITH_NULL_SHA"] = SSL_RSA_WITH_NULL_SHA;
        _ciphers["RSA_EXPORT_WITH_RC4_40_MD5"] = SSL_RSA_EXPORT_WITH_RC4_40_MD5;
        _ciphers["RSA_WITH_RC4_128_MD5"] = SSL_RSA_WITH_RC4_128_MD5;
        _ciphers["RSA_WITH_RC4_128_SHA"] = SSL_RSA_WITH_RC4_128_SHA;
        _ciphers["RSA_EXPORT_WITH_RC2_CBC_40_MD5"] =    SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5;
        _ciphers["RSA_WITH_IDEA_CBC_SHA"] = SSL_RSA_WITH_IDEA_CBC_SHA;
        _ciphers["RSA_EXPORT_WITH_DES40_CBC_SHA"] = SSL_RSA_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["RSA_WITH_DES_CBC_SHA"] = SSL_RSA_WITH_DES_CBC_SHA;
        _ciphers["RSA_WITH_3DES_EDE_CBC_SHA"] = SSL_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DH_DSS_EXPORT_WITH_DES40_CBC_SHA"] = SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["DH_DSS_WITH_DES_CBC_SHA"] = SSL_DH_DSS_WITH_DES_CBC_SHA;
        _ciphers["DH_DSS_WITH_3DES_EDE_CBC_SHA"] = SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DH_RSA_EXPORT_WITH_DES40_CBC_SHA"] = SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["DH_RSA_WITH_DES_CBC_SHA"] = SSL_DH_RSA_WITH_DES_CBC_SHA;
        _ciphers["DH_RSA_WITH_3DES_EDE_CBC_SHA"] = SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DHE_DSS_EXPORT_WITH_DES40_CBC_SHA"] = SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["DHE_DSS_WITH_DES_CBC_SHA"] = SSL_DHE_DSS_WITH_DES_CBC_SHA;
        _ciphers["DHE_DSS_WITH_3DES_EDE_CBC_SHA"] = SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DHE_RSA_EXPORT_WITH_DES40_CBC_SHA"] = SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["DHE_RSA_WITH_DES_CBC_SHA"] = SSL_DHE_RSA_WITH_DES_CBC_SHA;
        _ciphers["DHE_RSA_WITH_3DES_EDE_CBC_SHA"] = SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DH_anon_EXPORT_WITH_RC4_40_MD5"] = SSL_DH_anon_EXPORT_WITH_RC4_40_MD5;
        _ciphers["DH_anon_WITH_RC4_128_MD5"] = SSL_DH_anon_WITH_RC4_128_MD5;
        _ciphers["DH_anon_EXPORT_WITH_DES40_CBC_SHA"] = SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA;
        _ciphers["DH_anon_WITH_DES_CBC_SHA"] = SSL_DH_anon_WITH_DES_CBC_SHA;
        _ciphers["DH_anon_WITH_3DES_EDE_CBC_SHA"] = SSL_DH_anon_WITH_3DES_EDE_CBC_SHA;
        _ciphers["FORTEZZA_DMS_WITH_NULL_SHA"] = SSL_FORTEZZA_DMS_WITH_NULL_SHA;
        _ciphers["FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA"] = SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA;

        //
        // TLS addenda using AES, per RFC 3268
        //
        _ciphers["RSA_WITH_AES_128_CBC_SHA"] = TLS_RSA_WITH_AES_128_CBC_SHA;
        _ciphers["DH_DSS_WITH_AES_128_CBC_SHA"] = TLS_DH_DSS_WITH_AES_128_CBC_SHA;
        _ciphers["DH_RSA_WITH_AES_128_CBC_SHA"] = TLS_DH_RSA_WITH_AES_128_CBC_SHA;
        _ciphers["DHE_DSS_WITH_AES_128_CBC_SHA"] = TLS_DHE_DSS_WITH_AES_128_CBC_SHA;
        _ciphers["DHE_RSA_WITH_AES_128_CBC_SHA"] = TLS_DHE_RSA_WITH_AES_128_CBC_SHA;
        _ciphers["DH_anon_WITH_AES_128_CBC_SHA"] = TLS_DH_anon_WITH_AES_128_CBC_SHA;
        _ciphers["RSA_WITH_AES_256_CBC_SHA"] = TLS_RSA_WITH_AES_256_CBC_SHA;
        _ciphers["DH_DSS_WITH_AES_256_CBC_SHA"] = TLS_DH_DSS_WITH_AES_256_CBC_SHA;
        _ciphers["DH_RSA_WITH_AES_256_CBC_SHA"] = TLS_DH_RSA_WITH_AES_256_CBC_SHA;
        _ciphers["DHE_DSS_WITH_AES_256_CBC_SHA"] = TLS_DHE_DSS_WITH_AES_256_CBC_SHA;
        _ciphers["DHE_RSA_WITH_AES_256_CBC_SHA"] = TLS_DHE_RSA_WITH_AES_256_CBC_SHA;
        _ciphers["DH_anon_WITH_AES_256_CBC_SHA"] = TLS_DH_anon_WITH_AES_256_CBC_SHA;

        //
        // ECDSA addenda, RFC 4492
        //
        _ciphers["ECDH_ECDSA_WITH_NULL_SHA"] = TLS_ECDH_ECDSA_WITH_NULL_SHA;
        _ciphers["ECDH_ECDSA_WITH_RC4_128_SHA"] = TLS_ECDH_ECDSA_WITH_RC4_128_SHA;
        _ciphers["ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA"] = TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["ECDH_ECDSA_WITH_AES_128_CBC_SHA"] = TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA;
        _ciphers["ECDH_ECDSA_WITH_AES_256_CBC_SHA"] = TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA;
        _ciphers["ECDHE_ECDSA_WITH_NULL_SHA"] = TLS_ECDHE_ECDSA_WITH_NULL_SHA;
        _ciphers["ECDHE_ECDSA_WITH_RC4_128_SHA"] = TLS_ECDHE_ECDSA_WITH_RC4_128_SHA;
        _ciphers["ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA"] = TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["ECDHE_ECDSA_WITH_AES_128_CBC_SHA"] = TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA;
        _ciphers["ECDHE_ECDSA_WITH_AES_256_CBC_SHA"] = TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA;
        _ciphers["ECDH_RSA_WITH_NULL_SHA"] = TLS_ECDH_RSA_WITH_NULL_SHA;
        _ciphers["ECDH_RSA_WITH_RC4_128_SHA"] = TLS_ECDH_RSA_WITH_RC4_128_SHA;
        _ciphers["ECDH_RSA_WITH_3DES_EDE_CBC_SHA"] = TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["ECDH_RSA_WITH_AES_128_CBC_SHA"] = TLS_ECDH_RSA_WITH_AES_128_CBC_SHA;
        _ciphers["ECDH_RSA_WITH_AES_256_CBC_SHA"] = TLS_ECDH_RSA_WITH_AES_256_CBC_SHA;
        _ciphers["ECDHE_RSA_WITH_NULL_SHA"] = TLS_ECDHE_RSA_WITH_NULL_SHA;
        _ciphers["ECDHE_RSA_WITH_RC4_128_SHA"] = TLS_ECDHE_RSA_WITH_RC4_128_SHA;
        _ciphers["ECDHE_RSA_WITH_3DES_EDE_CBC_SHA"] = TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["ECDHE_RSA_WITH_AES_128_CBC_SHA"] = TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA;
        _ciphers["ECDHE_RSA_WITH_AES_256_CBC_SHA"] = TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA;
        _ciphers["ECDH_anon_WITH_NULL_SHA"] = TLS_ECDH_anon_WITH_NULL_SHA;
        _ciphers["ECDH_anon_WITH_RC4_128_SHA"] = TLS_ECDH_anon_WITH_RC4_128_SHA;
        _ciphers["ECDH_anon_WITH_3DES_EDE_CBC_SHA"] = TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA;
        _ciphers["ECDH_anon_WITH_AES_128_CBC_SHA"] = TLS_ECDH_anon_WITH_AES_128_CBC_SHA;
        _ciphers["ECDH_anon_WITH_AES_256_CBC_SHA"] = TLS_ECDH_anon_WITH_AES_256_CBC_SHA;

        //
        // TLS 1.2 addenda, RFC 5246
        //
        //_ciphers["NULL_WITH_NULL_NULL"] = TLS_NULL_WITH_NULL_NULL;

        //
        // Server provided RSA certificate for key exchange.
        //
        //_ciphers["RSA_WITH_NULL_MD5"] = TLS_RSA_WITH_NULL_MD5;
        //_ciphers["RSA_WITH_NULL_SHA"] = TLS_RSA_WITH_NULL_SHA;
        //_ciphers["RSA_WITH_RC4_128_MD5"] = TLS_RSA_WITH_RC4_128_MD5;
        //_ciphers["RSA_WITH_RC4_128_SHA"] = TLS_RSA_WITH_RC4_128_SHA;
        //_ciphers["RSA_WITH_3DES_EDE_CBC_SHA"] = TLS_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["RSA_WITH_NULL_SHA256"] = TLS_RSA_WITH_NULL_SHA256;
        _ciphers["RSA_WITH_AES_128_CBC_SHA256"] = TLS_RSA_WITH_AES_128_CBC_SHA256;
        _ciphers["RSA_WITH_AES_256_CBC_SHA256"] = TLS_RSA_WITH_AES_256_CBC_SHA256;

        //
        // Server-authenticated (and optionally client-authenticated) Diffie-Hellman.
        //
        //_ciphers["DH_DSS_WITH_3DES_EDE_CBC_SHA"] = TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA;
        //_ciphers["DH_RSA_WITH_3DES_EDE_CBC_SHA"] = TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA;
        //_ciphers["DHE_DSS_WITH_3DES_EDE_CBC_SHA"] = TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA;
        //_ciphers["DHE_RSA_WITH_3DES_EDE_CBC_SHA"] = TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DH_DSS_WITH_AES_128_CBC_SHA256"] = TLS_DH_DSS_WITH_AES_128_CBC_SHA256;
        _ciphers["DH_RSA_WITH_AES_128_CBC_SHA256"] = TLS_DH_RSA_WITH_AES_128_CBC_SHA256;
        _ciphers["DHE_DSS_WITH_AES_128_CBC_SHA256"] = TLS_DHE_DSS_WITH_AES_128_CBC_SHA256;
        _ciphers["DHE_RSA_WITH_AES_128_CBC_SHA256"] = TLS_DHE_RSA_WITH_AES_128_CBC_SHA256;
        _ciphers["DH_DSS_WITH_AES_256_CBC_SHA256"] = TLS_DH_DSS_WITH_AES_256_CBC_SHA256;
        _ciphers["DH_RSA_WITH_AES_256_CBC_SHA256"] = TLS_DH_RSA_WITH_AES_256_CBC_SHA256;
        _ciphers["DHE_DSS_WITH_AES_256_CBC_SHA256"] = TLS_DHE_DSS_WITH_AES_256_CBC_SHA256;
        _ciphers["DHE_RSA_WITH_AES_256_CBC_SHA256"] = TLS_DHE_RSA_WITH_AES_256_CBC_SHA256;

        //
        // Completely anonymous Diffie-Hellman
        //
        //_ciphers["DH_anon_WITH_RC4_128_MD5"] = TLS_DH_anon_WITH_RC4_128_MD5;
        //_ciphers["DH_anon_WITH_3DES_EDE_CBC_SHA"] = TLS_DH_anon_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DH_anon_WITH_AES_128_CBC_SHA256"] = TLS_DH_anon_WITH_AES_128_CBC_SHA256;
        _ciphers["DH_anon_WITH_AES_256_CBC_SHA256"] = TLS_DH_anon_WITH_AES_256_CBC_SHA256;

        //
        // Addendum from RFC 4279, TLS PSK
        //
        _ciphers["PSK_WITH_RC4_128_SHA"] = TLS_PSK_WITH_RC4_128_SHA;
        _ciphers["PSK_WITH_3DES_EDE_CBC_SHA"] = TLS_PSK_WITH_3DES_EDE_CBC_SHA;
        _ciphers["PSK_WITH_AES_128_CBC_SHA"] = TLS_PSK_WITH_AES_128_CBC_SHA;
        _ciphers["PSK_WITH_AES_256_CBC_SHA"] = TLS_PSK_WITH_AES_256_CBC_SHA;
        _ciphers["DHE_PSK_WITH_RC4_128_SHA"] = TLS_DHE_PSK_WITH_RC4_128_SHA;
        _ciphers["DHE_PSK_WITH_3DES_EDE_CBC_SHA"] = TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA;
        _ciphers["DHE_PSK_WITH_AES_128_CBC_SHA"] = TLS_DHE_PSK_WITH_AES_128_CBC_SHA;
        _ciphers["DHE_PSK_WITH_AES_256_CBC_SHA"] = TLS_DHE_PSK_WITH_AES_256_CBC_SHA;
        _ciphers["RSA_PSK_WITH_RC4_128_SHA"] = TLS_RSA_PSK_WITH_RC4_128_SHA;
        _ciphers["RSA_PSK_WITH_3DES_EDE_CBC_SHA"] = TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA;
        _ciphers["RSA_PSK_WITH_AES_128_CBC_SHA"] = TLS_RSA_PSK_WITH_AES_128_CBC_SHA;
        _ciphers["RSA_PSK_WITH_AES_256_CBC_SHA"] = TLS_RSA_PSK_WITH_AES_256_CBC_SHA;

        //
        // RFC 4785 - Pre-Shared Key (PSK) Ciphersuites with NULL Encryption
        //
        _ciphers["PSK_WITH_NULL_SHA"] = TLS_PSK_WITH_NULL_SHA;
        _ciphers["DHE_PSK_WITH_NULL_SHA"] = TLS_DHE_PSK_WITH_NULL_SHA;
        _ciphers["RSA_PSK_WITH_NULL_SHA"] = TLS_RSA_PSK_WITH_NULL_SHA;

        //
        // Addenda from rfc 5288 AES Galois Counter Mode (GCM) Cipher Suites for TLS.
        //
        _ciphers["RSA_WITH_AES_128_GCM_SHA256"] = TLS_RSA_WITH_AES_128_GCM_SHA256;
        _ciphers["RSA_WITH_AES_256_GCM_SHA384"] = TLS_RSA_WITH_AES_256_GCM_SHA384;
        _ciphers["DHE_RSA_WITH_AES_128_GCM_SHA256"] = TLS_DHE_RSA_WITH_AES_128_GCM_SHA256;
        _ciphers["DHE_RSA_WITH_AES_256_GCM_SHA384"] = TLS_DHE_RSA_WITH_AES_256_GCM_SHA384;
        _ciphers["DH_RSA_WITH_AES_128_GCM_SHA256"] = TLS_DH_RSA_WITH_AES_128_GCM_SHA256;
        _ciphers["DH_RSA_WITH_AES_256_GCM_SHA384"] = TLS_DH_RSA_WITH_AES_256_GCM_SHA384;
        _ciphers["DHE_DSS_WITH_AES_128_GCM_SHA256"] = TLS_DHE_DSS_WITH_AES_128_GCM_SHA256;
        _ciphers["DHE_DSS_WITH_AES_256_GCM_SHA384"] = TLS_DHE_DSS_WITH_AES_256_GCM_SHA384;
        _ciphers["DH_DSS_WITH_AES_128_GCM_SHA256"] = TLS_DH_DSS_WITH_AES_128_GCM_SHA256;
        _ciphers["DH_DSS_WITH_AES_256_GCM_SHA384"] = TLS_DH_DSS_WITH_AES_256_GCM_SHA384;
        _ciphers["DH_anon_WITH_AES_128_GCM_SHA256"] = TLS_DH_anon_WITH_AES_128_GCM_SHA256;
        _ciphers["DH_anon_WITH_AES_256_GCM_SHA384"] = TLS_DH_anon_WITH_AES_256_GCM_SHA384;

        //
        // RFC 5487 - PSK with SHA-256/384 and AES GCM
        //
        _ciphers["PSK_WITH_AES_128_GCM_SHA256"] = TLS_PSK_WITH_AES_128_GCM_SHA256;
        _ciphers["PSK_WITH_AES_256_GCM_SHA384"] = TLS_PSK_WITH_AES_256_GCM_SHA384;
        _ciphers["DHE_PSK_WITH_AES_128_GCM_SHA256"] = TLS_DHE_PSK_WITH_AES_128_GCM_SHA256;
        _ciphers["DHE_PSK_WITH_AES_256_GCM_SHA384"] = TLS_DHE_PSK_WITH_AES_256_GCM_SHA384;
        _ciphers["RSA_PSK_WITH_AES_128_GCM_SHA256"] = TLS_RSA_PSK_WITH_AES_128_GCM_SHA256;
        _ciphers["RSA_PSK_WITH_AES_256_GCM_SHA384"] = TLS_RSA_PSK_WITH_AES_256_GCM_SHA384;

        _ciphers["PSK_WITH_AES_128_CBC_SHA256"] = TLS_PSK_WITH_AES_128_CBC_SHA256;
        _ciphers["PSK_WITH_AES_256_CBC_SHA384"] = TLS_PSK_WITH_AES_256_CBC_SHA384;
        _ciphers["PSK_WITH_NULL_SHA256"] = TLS_PSK_WITH_NULL_SHA256;
        _ciphers["PSK_WITH_NULL_SHA384"] = TLS_PSK_WITH_NULL_SHA384;

        _ciphers["DHE_PSK_WITH_AES_128_CBC_SHA256"] = TLS_DHE_PSK_WITH_AES_128_CBC_SHA256;
        _ciphers["DHE_PSK_WITH_AES_256_CBC_SHA384"] = TLS_DHE_PSK_WITH_AES_256_CBC_SHA384;
        _ciphers["DHE_PSK_WITH_NULL_SHA256"] = TLS_DHE_PSK_WITH_NULL_SHA256;
        _ciphers["DHE_PSK_WITH_NULL_SHA384"] = TLS_DHE_PSK_WITH_NULL_SHA384;

        _ciphers["RSA_PSK_WITH_AES_128_CBC_SHA256"] = TLS_RSA_PSK_WITH_AES_128_CBC_SHA256;
        _ciphers["RSA_PSK_WITH_AES_256_CBC_SHA384"] = TLS_RSA_PSK_WITH_AES_256_CBC_SHA384;
        _ciphers["RSA_PSK_WITH_NULL_SHA256"] = TLS_RSA_PSK_WITH_NULL_SHA256;
        _ciphers["RSA_PSK_WITH_NULL_SHA384"] = TLS_RSA_PSK_WITH_NULL_SHA384;

        //
        // Addenda from rfc 5289  Elliptic Curve Cipher Suites with HMAC SHA-256/384.
        //
        _ciphers["ECDHE_ECDSA_WITH_AES_128_CBC_SHA256"] = TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256;
        _ciphers["ECDHE_ECDSA_WITH_AES_256_CBC_SHA384"] = TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384;
        _ciphers["ECDH_ECDSA_WITH_AES_128_CBC_SHA256"] = TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256;
        _ciphers["ECDH_ECDSA_WITH_AES_256_CBC_SHA384"] = TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384;
        _ciphers["ECDHE_RSA_WITH_AES_128_CBC_SHA256"] = TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256;
        _ciphers["ECDHE_RSA_WITH_AES_256_CBC_SHA384"] = TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384;
        _ciphers["ECDH_RSA_WITH_AES_128_CBC_SHA256"] = TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256;
        _ciphers["ECDH_RSA_WITH_AES_256_CBC_SHA384"] = TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384;

        //
        // Addenda from rfc 5289  Elliptic Curve Cipher Suites with SHA-256/384 and AES Galois Counter Mode (GCM)
        //
        _ciphers["ECDHE_ECDSA_WITH_AES_128_GCM_SHA256"] = TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256;
        _ciphers["ECDHE_ECDSA_WITH_AES_256_GCM_SHA384"] = TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384;
        _ciphers["ECDH_ECDSA_WITH_AES_128_GCM_SHA256"] = TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256;
        _ciphers["ECDH_ECDSA_WITH_AES_256_GCM_SHA384"] = TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384;
        _ciphers["ECDHE_RSA_WITH_AES_128_GCM_SHA256"] = TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256;
        _ciphers["ECDHE_RSA_WITH_AES_256_GCM_SHA384"] = TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384;
        _ciphers["ECDH_RSA_WITH_AES_128_GCM_SHA256"] = TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256;
        _ciphers["ECDH_RSA_WITH_AES_256_GCM_SHA384"] = TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384;

        //
        // RFC 5746 - Secure Renegotiation
        //
        _ciphers["EMPTY_RENEGOTIATION_INFO_SCSV"] = TLS_EMPTY_RENEGOTIATION_INFO_SCSV;

        //
        // Tags for SSL 2 cipher kinds that are not specified for SSL 3.
        //
        _ciphers["RSA_WITH_RC2_CBC_MD5"] = SSL_RSA_WITH_RC2_CBC_MD5;
        _ciphers["RSA_WITH_IDEA_CBC_MD5"] = SSL_RSA_WITH_IDEA_CBC_MD5;
        _ciphers["RSA_WITH_DES_CBC_MD5"] = SSL_RSA_WITH_DES_CBC_MD5;
        _ciphers["RSA_WITH_3DES_EDE_CBC_MD5"] = SSL_RSA_WITH_3DES_EDE_CBC_MD5;
        _ciphers["NO_SUCH_CIPHERSUITE"] = SSL_NO_SUCH_CIPHERSUITE;
    }
}

SSLCipherSuite
CiphersHelper::cipherForName(const string& name)
{
    map<string, SSLCipherSuite>::const_iterator i = _ciphers.find(name);
    if(i == _ciphers.end() || i->second == SSL_NO_SUCH_CIPHERSUITE)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: no such cipher " + name);
    }
    return i->second;
}

//
// Retrive the name of a cipher, SSLCipherSuite inlude duplicated values for TLS/SSL
// protocol ciphers, for example SSL_RSA_WITH_RC4_128_MD5/TLS_RSA_WITH_RC4_128_MD5
// are represeted by the same SSLCipherSuite value, the names return by this method
// doesn't include a protocol prefix.
//
string
CiphersHelper::cipherName(SSLCipherSuite cipher)
{
    switch(cipher)
    {
        case SSL_NULL_WITH_NULL_NULL:
            return "NULL_WITH_NULL_NULL";
        case SSL_RSA_WITH_NULL_MD5:
            return "RSA_WITH_NULL_MD5";
        case SSL_RSA_WITH_NULL_SHA:
            return "RSA_WITH_NULL_SHA";
        case SSL_RSA_EXPORT_WITH_RC4_40_MD5:
            return "RSA_EXPORT_WITH_RC4_40_MD5";
        case SSL_RSA_WITH_RC4_128_MD5:
            return "RSA_WITH_RC4_128_MD5";
        case SSL_RSA_WITH_RC4_128_SHA:
            return "RSA_WITH_RC4_128_SHA";
        case SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5:
            return "RSA_EXPORT_WITH_RC2_CBC_40_MD5";
        case SSL_RSA_WITH_IDEA_CBC_SHA:
            return "RSA_WITH_IDEA_CBC_SHA";
        case SSL_RSA_EXPORT_WITH_DES40_CBC_SHA:
            return "RSA_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_RSA_WITH_DES_CBC_SHA:
            return "RSA_WITH_DES_CBC_SHA";
        case SSL_RSA_WITH_3DES_EDE_CBC_SHA:
            return "RSA_WITH_3DES_EDE_CBC_SHA";
        case SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA:
            return "DH_DSS_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_DH_DSS_WITH_DES_CBC_SHA:
            return "DH_DSS_WITH_DES_CBC_SHA";
        case SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA:
            return "DH_DSS_WITH_3DES_EDE_CBC_SHA";
        case SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA:
            return "DH_RSA_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_DH_RSA_WITH_DES_CBC_SHA:
            return "DH_RSA_WITH_DES_CBC_SHA";
        case SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA:
            return "DH_RSA_WITH_3DES_EDE_CBC_SHA";
        case SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA:
            return "DHE_DSS_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_DHE_DSS_WITH_DES_CBC_SHA:
            return "DHE_DSS_WITH_DES_CBC_SHA";
        case SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA:
            return "DHE_DSS_WITH_3DES_EDE_CBC_SHA";
        case SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA:
            return "DHE_RSA_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_DHE_RSA_WITH_DES_CBC_SHA:
            return "DHE_RSA_WITH_DES_CBC_SHA";
        case SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
            return "DHE_RSA_WITH_3DES_EDE_CBC_SHA";
        case SSL_DH_anon_EXPORT_WITH_RC4_40_MD5:
            return "DH_anon_EXPORT_WITH_RC4_40_MD5";
        case SSL_DH_anon_WITH_RC4_128_MD5:
            return "DH_anon_WITH_RC4_128_MD5";
        case SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA:
            return "DH_anon_EXPORT_WITH_DES40_CBC_SHA";
        case SSL_DH_anon_WITH_DES_CBC_SHA:
            return "DH_anon_WITH_DES_CBC_SHA";
        case SSL_DH_anon_WITH_3DES_EDE_CBC_SHA:
            return "DH_anon_WITH_3DES_EDE_CBC_SHA";
        case SSL_FORTEZZA_DMS_WITH_NULL_SHA:
            return "FORTEZZA_DMS_WITH_NULL_SHA";
        case SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA:
            return "FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA";

        //
        // TLS addenda using AES, per RFC 3268
        //
        case TLS_RSA_WITH_AES_128_CBC_SHA:
            return "RSA_WITH_AES_128_CBC_SHA";
        case TLS_DH_DSS_WITH_AES_128_CBC_SHA:
            return "DH_DSS_WITH_AES_128_CBC_SHA";
        case TLS_DH_RSA_WITH_AES_128_CBC_SHA:
            return "DH_RSA_WITH_AES_128_CBC_SHA";
        case TLS_DHE_DSS_WITH_AES_128_CBC_SHA:
            return "DHE_DSS_WITH_AES_128_CBC_SHA";
        case TLS_DHE_RSA_WITH_AES_128_CBC_SHA:
            return "DHE_RSA_WITH_AES_128_CBC_SHA";
        case TLS_DH_anon_WITH_AES_128_CBC_SHA:
            return "DH_anon_WITH_AES_128_CBC_SHA";
        case TLS_RSA_WITH_AES_256_CBC_SHA:
            return "RSA_WITH_AES_256_CBC_SHA";
        case TLS_DH_DSS_WITH_AES_256_CBC_SHA:
            return "DH_DSS_WITH_AES_256_CBC_SHA";
        case TLS_DH_RSA_WITH_AES_256_CBC_SHA:
            return "DH_RSA_WITH_AES_256_CBC_SHA";
        case TLS_DHE_DSS_WITH_AES_256_CBC_SHA:
            return "DHE_DSS_WITH_AES_256_CBC_SHA";
        case TLS_DHE_RSA_WITH_AES_256_CBC_SHA:
            return "DHE_RSA_WITH_AES_256_CBC_SHA";
        case TLS_DH_anon_WITH_AES_256_CBC_SHA:
            return "DH_anon_WITH_AES_256_CBC_SHA";

        //
        // ECDSA addenda, RFC 4492
        //
        case TLS_ECDH_ECDSA_WITH_NULL_SHA:
            return "ECDH_ECDSA_WITH_NULL_SHA";
        case TLS_ECDH_ECDSA_WITH_RC4_128_SHA:
            return "ECDH_ECDSA_WITH_RC4_128_SHA";
        case TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA:
            return "ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA:
            return "ECDH_ECDSA_WITH_AES_128_CBC_SHA";
        case TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA:
            return "ECDH_ECDSA_WITH_AES_256_CBC_SHA";
        case TLS_ECDHE_ECDSA_WITH_NULL_SHA:
            return "ECDHE_ECDSA_WITH_NULL_SHA";
        case TLS_ECDHE_ECDSA_WITH_RC4_128_SHA:
            return "ECDHE_ECDSA_WITH_RC4_128_SHA";
        case TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA:
            return "ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA:
            return "ECDHE_ECDSA_WITH_AES_128_CBC_SHA";
        case TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA:
            return "ECDHE_ECDSA_WITH_AES_256_CBC_SHA";
        case TLS_ECDH_RSA_WITH_NULL_SHA:
            return "ECDH_RSA_WITH_NULL_SHA";
        case TLS_ECDH_RSA_WITH_RC4_128_SHA:
            return "ECDH_RSA_WITH_RC4_128_SHA";
        case TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA:
            return "ECDH_RSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_ECDH_RSA_WITH_AES_128_CBC_SHA:
            return "ECDH_RSA_WITH_AES_128_CBC_SHA";
        case TLS_ECDH_RSA_WITH_AES_256_CBC_SHA:
            return "ECDH_RSA_WITH_AES_256_CBC_SHA";
        case TLS_ECDHE_RSA_WITH_NULL_SHA:
            return "ECDHE_RSA_WITH_NULL_SHA";
        case TLS_ECDHE_RSA_WITH_RC4_128_SHA:
            return "ECDHE_RSA_WITH_RC4_128_SHA";
        case TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA:
            return "ECDHE_RSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA:
            return "ECDHE_RSA_WITH_AES_128_CBC_SHA";
        case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA:
            return "ECDHE_RSA_WITH_AES_256_CBC_SHA";
        case TLS_ECDH_anon_WITH_NULL_SHA:
            return "ECDH_anon_WITH_NULL_SHA";
        case TLS_ECDH_anon_WITH_RC4_128_SHA:
            return "ECDH_anon_WITH_RC4_128_SHA";
        case TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA:
            return "ECDH_anon_WITH_3DES_EDE_CBC_SHA";
        case TLS_ECDH_anon_WITH_AES_128_CBC_SHA:
            return "ECDH_anon_WITH_AES_128_CBC_SHA";
        case TLS_ECDH_anon_WITH_AES_256_CBC_SHA:
            return "ECDH_anon_WITH_AES_256_CBC_SHA";

        //
        // TLS 1.2 addenda, RFC 5246
        //
        //case TLS_NULL_WITH_NULL_NULL:
        //    return "NULL_WITH_NULL_NULL";

        //
        // Server provided RSA certificate for key exchange.
        //
        //case TLS_RSA_WITH_NULL_MD5:
        //    return "RSA_WITH_NULL_MD5";
        //case TLS_RSA_WITH_NULL_SHA:
        //    return "RSA_WITH_NULL_SHA";
        //case TLS_RSA_WITH_RC4_128_MD5:
        //    return "RSA_WITH_RC4_128_MD5";
        //case TLS_RSA_WITH_RC4_128_SHA:
        //    return "RSA_WITH_RC4_128_SHA";
        //case TLS_RSA_WITH_3DES_EDE_CBC_SHA:
        //    return "RSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_RSA_WITH_NULL_SHA256:
            return "RSA_WITH_NULL_SHA256";
        case TLS_RSA_WITH_AES_128_CBC_SHA256:
            return "RSA_WITH_AES_128_CBC_SHA256";
        case TLS_RSA_WITH_AES_256_CBC_SHA256:
            return "RSA_WITH_AES_256_CBC_SHA256";

        //
        // Server-authenticated (and optionally client-authenticated) Diffie-Hellman.
        //
        //case TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA:
        //    return "DH_DSS_WITH_3DES_EDE_CBC_SHA";
        //case TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA:
        //    return "DH_RSA_WITH_3DES_EDE_CBC_SHA";
        //case TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA:
        //    return "DHE_DSS_WITH_3DES_EDE_CBC_SHA";
        //case TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
        //    return "DHE_RSA_WITH_3DES_EDE_CBC_SHA";
        case TLS_DH_DSS_WITH_AES_128_CBC_SHA256:
            return "DH_DSS_WITH_AES_128_CBC_SHA256";
        case TLS_DH_RSA_WITH_AES_128_CBC_SHA256:
            return "DH_RSA_WITH_AES_128_CBC_SHA256";
        case TLS_DHE_DSS_WITH_AES_128_CBC_SHA256:
            return "DHE_DSS_WITH_AES_128_CBC_SHA256";
        case TLS_DHE_RSA_WITH_AES_128_CBC_SHA256:
            return "DHE_RSA_WITH_AES_128_CBC_SHA256";
        case TLS_DH_DSS_WITH_AES_256_CBC_SHA256:
            return "DH_DSS_WITH_AES_256_CBC_SHA256";
        case TLS_DH_RSA_WITH_AES_256_CBC_SHA256:
            return "DH_RSA_WITH_AES_256_CBC_SHA256";
        case TLS_DHE_DSS_WITH_AES_256_CBC_SHA256:
            return "DHE_DSS_WITH_AES_256_CBC_SHA256";
        case TLS_DHE_RSA_WITH_AES_256_CBC_SHA256:
            return "DHE_RSA_WITH_AES_256_CBC_SHA256";

        //
        // Completely anonymous Diffie-Hellman
        //
        //case TLS_DH_anon_WITH_RC4_128_MD5:
        //    return "DH_anon_WITH_RC4_128_MD5";
        //case TLS_DH_anon_WITH_3DES_EDE_CBC_SHA:
        //    return "DH_anon_WITH_3DES_EDE_CBC_SHA";
        case TLS_DH_anon_WITH_AES_128_CBC_SHA256:
            return "DH_anon_WITH_AES_128_CBC_SHA256";
        case TLS_DH_anon_WITH_AES_256_CBC_SHA256:
            return "DH_anon_WITH_AES_256_CBC_SHA256";

        //
        // Addendum from RFC 4279, TLS PSK
        //
        case TLS_PSK_WITH_RC4_128_SHA:
            return "PSK_WITH_RC4_128_SHA";
        case TLS_PSK_WITH_3DES_EDE_CBC_SHA:
            return "PSK_WITH_3DES_EDE_CBC_SHA";
        case TLS_PSK_WITH_AES_128_CBC_SHA:
            return "PSK_WITH_AES_128_CBC_SHA";
        case TLS_PSK_WITH_AES_256_CBC_SHA:
            return "PSK_WITH_AES_256_CBC_SHA";
        case TLS_DHE_PSK_WITH_RC4_128_SHA:
            return "DHE_PSK_WITH_RC4_128_SHA";
        case TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA:
            return "DHE_PSK_WITH_3DES_EDE_CBC_SHA";
        case TLS_DHE_PSK_WITH_AES_128_CBC_SHA:
            return "DHE_PSK_WITH_AES_128_CBC_SHA";
        case TLS_DHE_PSK_WITH_AES_256_CBC_SHA:
            return "DHE_PSK_WITH_AES_256_CBC_SHA";
        case TLS_RSA_PSK_WITH_RC4_128_SHA:
            return "RSA_PSK_WITH_RC4_128_SHA";
        case TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA:
            return "RSA_PSK_WITH_3DES_EDE_CBC_SHA";
        case TLS_RSA_PSK_WITH_AES_128_CBC_SHA:
            return "RSA_PSK_WITH_AES_128_CBC_SHA";
        case TLS_RSA_PSK_WITH_AES_256_CBC_SHA:
            return "RSA_PSK_WITH_AES_256_CBC_SHA";

        //
        // RFC 4785 - Pre-Shared Key (PSK) Ciphersuites with NULL Encryption
        //
        case TLS_PSK_WITH_NULL_SHA:
            return "PSK_WITH_NULL_SHA";
        case TLS_DHE_PSK_WITH_NULL_SHA:
            return "DHE_PSK_WITH_NULL_SHA";
        case TLS_RSA_PSK_WITH_NULL_SHA:
            return "RSA_PSK_WITH_NULL_SHA";

        //
        // Addenda from rfc 5288 AES Galois Counter Mode (GCM) Cipher Suites for TLS.
        //
        case TLS_RSA_WITH_AES_128_GCM_SHA256:
            return "RSA_WITH_AES_128_GCM_SHA256";
        case TLS_RSA_WITH_AES_256_GCM_SHA384:
            return "RSA_WITH_AES_256_GCM_SHA384";
        case TLS_DHE_RSA_WITH_AES_128_GCM_SHA256:
            return "DHE_RSA_WITH_AES_128_GCM_SHA256";
        case TLS_DHE_RSA_WITH_AES_256_GCM_SHA384:
            return "DHE_RSA_WITH_AES_256_GCM_SHA384";
        case TLS_DH_RSA_WITH_AES_128_GCM_SHA256:
            return "DH_RSA_WITH_AES_128_GCM_SHA256";
        case TLS_DH_RSA_WITH_AES_256_GCM_SHA384:
            return "DH_RSA_WITH_AES_256_GCM_SHA384";
        case TLS_DHE_DSS_WITH_AES_128_GCM_SHA256:
            return "DHE_DSS_WITH_AES_128_GCM_SHA256";
        case TLS_DHE_DSS_WITH_AES_256_GCM_SHA384:
            return "DHE_DSS_WITH_AES_256_GCM_SHA384";
        case TLS_DH_DSS_WITH_AES_128_GCM_SHA256:
            return "DH_DSS_WITH_AES_128_GCM_SHA256";
        case TLS_DH_DSS_WITH_AES_256_GCM_SHA384:
            return "DH_DSS_WITH_AES_256_GCM_SHA384";
        case TLS_DH_anon_WITH_AES_128_GCM_SHA256:
            return "DH_anon_WITH_AES_128_GCM_SHA256";
        case TLS_DH_anon_WITH_AES_256_GCM_SHA384:
            return "DH_anon_WITH_AES_256_GCM_SHA384";

        //
        // RFC 5487 - PSK with SHA-256/384 and AES GCM
        //
        case TLS_PSK_WITH_AES_128_GCM_SHA256:
            return "PSK_WITH_AES_128_GCM_SHA256";
        case TLS_PSK_WITH_AES_256_GCM_SHA384:
            return "PSK_WITH_AES_256_GCM_SHA384";
        case TLS_DHE_PSK_WITH_AES_128_GCM_SHA256:
            return "DHE_PSK_WITH_AES_128_GCM_SHA256";
        case TLS_DHE_PSK_WITH_AES_256_GCM_SHA384:
            return "DHE_PSK_WITH_AES_256_GCM_SHA384";
        case TLS_RSA_PSK_WITH_AES_128_GCM_SHA256:
            return "RSA_PSK_WITH_AES_128_GCM_SHA256";
        case TLS_RSA_PSK_WITH_AES_256_GCM_SHA384:
            return "RSA_PSK_WITH_AES_256_GCM_SHA384";

        case TLS_PSK_WITH_AES_128_CBC_SHA256:
            return "PSK_WITH_AES_128_CBC_SHA256";
        case TLS_PSK_WITH_AES_256_CBC_SHA384:
            return "PSK_WITH_AES_256_CBC_SHA384";
        case TLS_PSK_WITH_NULL_SHA256:
            return "WITH_NULL_SHA256";
        case TLS_PSK_WITH_NULL_SHA384:
            return "PSK_WITH_NULL_SHA384";

        case TLS_DHE_PSK_WITH_AES_128_CBC_SHA256:
            return "DHE_PSK_WITH_AES_128_CBC_SHA256";
        case TLS_DHE_PSK_WITH_AES_256_CBC_SHA384:
            return "DHE_PSK_WITH_AES_256_CBC_SHA384";
        case TLS_DHE_PSK_WITH_NULL_SHA256:
            return "DHE_PSK_WITH_NULL_SHA256";
        case TLS_DHE_PSK_WITH_NULL_SHA384:
            return "DHE_PSK_WITH_NULL_SHA384";

        case TLS_RSA_PSK_WITH_AES_128_CBC_SHA256:
            return "RSA_PSK_WITH_AES_128_CBC_SHA256";
        case TLS_RSA_PSK_WITH_AES_256_CBC_SHA384:
            return "RSA_PSK_WITH_AES_256_CBC_SHA384";
        case TLS_RSA_PSK_WITH_NULL_SHA256:
            return "RSA_PSK_WITH_NULL_SHA256";
        case TLS_RSA_PSK_WITH_NULL_SHA384:
            return "RSA_PSK_WITH_NULL_SHA384";

        //
        // Addenda from rfc 5289  Elliptic Curve Cipher Suites with HMAC SHA-256/384.
        //
        case TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256:
            return "ECDHE_ECDSA_WITH_AES_128_CBC_SHA256";
        case TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384:
            return "ECDHE_ECDSA_WITH_AES_256_CBC_SHA384";
        case TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256:
            return "ECDH_ECDSA_WITH_AES_128_CBC_SHA256";
        case TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384:
            return "ECDH_ECDSA_WITH_AES_256_CBC_SHA384";
        case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256:
            return "ECDHE_RSA_WITH_AES_128_CBC_SHA256";
        case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384:
            return "ECDHE_RSA_WITH_AES_256_CBC_SHA384";
        case TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256:
            return "ECDH_RSA_WITH_AES_128_CBC_SHA256";
        case TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384:
            return "ECDH_RSA_WITH_AES_256_CBC_SHA384";

        //
        // Addenda from rfc 5289  Elliptic Curve Cipher Suites with SHA-256/384 and AES Galois Counter Mode (GCM)
        //
        case TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256:
            return "ECDHE_ECDSA_WITH_AES_128_GCM_SHA256";
        case TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384:
            return "ECDHE_ECDSA_WITH_AES_256_GCM_SHA384";
        case TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256:
            return "ECDH_ECDSA_WITH_AES_128_GCM_SHA256";
        case TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384:
            return "ECDH_ECDSA_WITH_AES_256_GCM_SHA384";
        case TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256:
            return "ECDHE_RSA_WITH_AES_128_GCM_SHA256";
        case TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384:
            return "ECDHE_RSA_WITH_AES_256_GCM_SHA384";
        case TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256:
            return "ECDH_RSA_WITH_AES_128_GCM_SHA256";
        case TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384:
            return "ECDH_RSA_WITH_AES_256_GCM_SHA384";

        //
        // RFC 5746 - Secure Renegotiation
        //
        case TLS_EMPTY_RENEGOTIATION_INFO_SCSV:
            return "EMPTY_RENEGOTIATION_INFO_SCSV";

        //
        // Tags for SSL 2 cipher kinds that are not specified for SSL 3.
        //
        case SSL_RSA_WITH_RC2_CBC_MD5:
            return "RSA_WITH_RC2_CBC_MD5";
        case SSL_RSA_WITH_IDEA_CBC_MD5:
            return "RSA_WITH_IDEA_CBC_MD5";
        case SSL_RSA_WITH_DES_CBC_MD5:
            return "RSA_WITH_DES_CBC_MD5";
        case SSL_RSA_WITH_3DES_EDE_CBC_MD5:
            return "RSA_WITH_3DES_EDE_CBC_MD5";
        default:
            return "";
    }
}

map<string, SSLCipherSuite>
CiphersHelper::ciphers()
{
    return _ciphers;
}

SSLProtocol
parseProtocol(const string& p)
{
    const string prot = IceUtilInternal::toUpper(p);
    if(prot == "SSL3" || prot == "SSLV3")
    {
        return kSSLProtocol3;
    }
    else if(prot == "TLS" || prot == "TLS1" || prot == "TLSV1" || prot == "TLS1_0" || prot == "TLSV1_0")
    {
        return kTLSProtocol1;
    }
    else if(prot == "TLS1_1" || prot == "TLSV1_1")
    {
        return kTLSProtocol11;
    }
    else if(prot == "TLS1_2" || prot == "TLSV1_2")
    {
        return kTLSProtocol12;
    }
    else
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unrecognized protocol `" + p + "'");
    }
}

}

IceUtil::Shared* IceSSL::upCast(IceSSL::SecureTransportEngine* p) { return p; }

IceSSL::SecureTransportEngine::SecureTransportEngine(const Ice::CommunicatorPtr& communicator) :
    SSLEngine(communicator),
    _initialized(false),
    _certificateAuthorities(0),
    _chain(0),
    _protocolVersionMax(kSSLProtocolUnknown),
    _protocolVersionMin(kSSLProtocolUnknown)
{
}

bool
IceSSL::SecureTransportEngine::initialized() const
{
    IceUtil::Mutex::Lock lock(_mutex);
    return _initialized;
}

//
// Setup the engine.
//
void
IceSSL::SecureTransportEngine::initialize()
{
    IceUtil::Mutex::Lock lock(_mutex);
    if(_initialized)
    {
        return;
    }

    SSLEngine::initialize();

    const PropertiesPtr properties = communicator()->getProperties();

    //
    // Check for a default directory. We look in this directory for
    // files mentioned in the configuration.
    //
    const string defaultDir = properties->getProperty("IceSSL.DefaultDir");

    //
    // Load the CA certificates used to authenticate peers into
    // _certificateAuthorities array.
    //
    try
    {
        string caFile = properties->getProperty("IceSSL.CAs");
        if(caFile.empty())
        {
            caFile = properties->getProperty("IceSSL.CertAuthFile");
        }
        if(!caFile.empty())
        {
            string resolved;
            if(!checkPath(caFile, defaultDir, false, resolved))
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: CA certificate file not found:\n" + caFile);
            }
            _certificateAuthorities.reset(loadCACertificates(resolved));
        }
        else if(properties->getPropertyAsInt("IceSSL.UsePlatformCAs") <= 0)
        {
            // Setup an empty list of Root CAs to not use the system root CAs.
            _certificateAuthorities.reset(CFArrayCreate(0, 0, 0, 0));
        }
    }
    catch(const CertificateReadException& ce)
    {
        throw PluginInitializationException(__FILE__, __LINE__, ce.reason);
    }

    const string password = properties->getProperty("IceSSL.Password");
    const int passwordRetryMax = properties->getPropertyAsIntWithDefault("IceSSL.PasswordRetryMax", 3);
    PasswordPromptPtr passwordPrompt = getPasswordPrompt();

    string certFile = properties->getProperty("IceSSL.CertFile");
    string keyFile = properties->getProperty("IceSSL.KeyFile");
    string findCert = properties->getProperty("IceSSL.FindCert");
    if(!certFile.empty())
    {
        vector<string> files;
        if(!IceUtilInternal::splitString(certFile, IceUtilInternal::pathsep, files) || files.size() > 2)
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: invalid value for IceSSL.CertFile:\n" + certFile);
        }
        vector<string> keyFiles;
        if(!keyFile.empty())
        {
            if(!IceUtilInternal::splitString(keyFile, IceUtilInternal::pathsep, keyFiles) || keyFiles.size() > 2)
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: invalid value for IceSSL.KeyFile:\n" + keyFile);
            }
            if(files.size() != keyFiles.size())
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: IceSSL.KeyFile does not agree with IceSSL.CertFile");
            }
        }

        for(int i = 0; i < files.size(); ++i)
        {
            string file = files[i];
            string keyFile = keyFiles.empty() ? "" : keyFiles[i];
            string resolved;

            if(!checkPath(file, defaultDir, false, resolved))
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: certificate file not found:\n" + file);
            }
            file = resolved;

            if(!keyFile.empty())
            {
                if(!checkPath(keyFile, defaultDir, false, resolved))
                {
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: key file not found:\n" + keyFile);
                }
                keyFile = resolved;
            }

            UniqueRef<SecKeychainRef> kc(openKeychain());
            try
            {
                _chain.reset(loadCertificateChain(file, keyFile, kc.get(), password, passwordPrompt, passwordRetryMax));
                break;
            }
            catch(const CertificateReadException& ce)
            {
                //
                // If this is the last certificate rethrow the exception as PluginInitializationException,
                // otherwise try the next certificate.
                //
                if(i == files.size() - 1)
                {
                    throw PluginInitializationException(__FILE__, __LINE__, ce.reason);
                }
            }
        }
    }
    else if(!findCert.empty())
    {
        UniqueRef<SecKeychainRef> kc(openKeychain());
        UniqueRef<SecCertificateRef> cert(findCertificate(kc.get(), findCert));

        //
        // Retrieve the certificate chain
        //
        UniqueRef<SecPolicyRef> policy(SecPolicyCreateSSL(true, 0));
        SecTrustRef trust = 0;
        OSStatus err = SecTrustCreateWithCertificates((CFArrayRef)cert.get(), policy.get(), &trust);
        if(err || !trust)
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: error creating trust object" +
                                                (err ? ":\n" + errorToString(err) : ""));
        }
        UniqueRef<SecTrustRef> v(trust);

        SecTrustResultType trustResult;
        if((err = SecTrustEvaluate(trust, &trustResult)))
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: error evaluating trust:\n" + errorToString(err));
        }

        int chainLength = SecTrustGetCertificateCount(trust);
        CFMutableArrayRef items = CFArrayCreateMutable(kCFAllocatorDefault, chainLength, &kCFTypeArrayCallBacks);
        for(int i = 0; i < chainLength; ++i)
        {
            CFArrayAppendValue(items, SecTrustGetCertificateAtIndex(trust, i));
        }

        //
        // Replace the first certificate in the chain with the
        // identity.
        //
        SecIdentityRef identity;
        err = SecIdentityCreateWithCertificate(kc.get(), cert.get(), &identity);
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: couldn't create identity for certificate found in the keychain:\n" << errorToString(err);
            throw PluginInitializationException(__FILE__, __LINE__, os.str());
        }
        CFArraySetValueAtIndex(items, 0, identity);
        CFRelease(identity);
        _chain.reset(items);
    }

    //
    // DiffieHellmanParams in DER format.
    //
    string dhFile = properties->getProperty("IceSSL.DHParams");
    if(!dhFile.empty())
    {
        string resolved;
        if(!checkPath(dhFile, defaultDir, false, resolved))
        {
            throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: DH params file not found:\n" + dhFile);
        }

        readFile(resolved, _dhParams);
    }

    //
    // Establish the cipher list.
    //
    const string ciphers = properties->getProperty("IceSSL.Ciphers");
    CiphersHelper::initialize();

    if(!ciphers.empty())
    {
        parseCiphers(ciphers);
    }

    if(securityTraceLevel() >= 1)
    {
        ostringstream os;
        os << "enabling SSL ciphersuites:";

        if(_ciphers.empty())
        {
            map<string, SSLCipherSuite> enabled = CiphersHelper::ciphers();
            for(map<string, SSLCipherSuite>::const_iterator i = enabled.begin(); i != enabled.end(); ++i)
            {
                os << "\n " << i->first;
            }
        }
        else
        {
            for(vector<SSLCipherSuite>::const_iterator i = _ciphers.begin(); i != _ciphers.end(); ++i)
            {
                os << "\n " << getCipherName(*i);
            }
        }
        getLogger()->trace(securityTraceCategory(), os.str());
    }

    //
    // Parse protocols
    //
    const string protocolVersionMax = properties->getProperty("IceSSL.ProtocolVersionMax");
    if(!protocolVersionMax.empty())
    {
        _protocolVersionMax = parseProtocol(protocolVersionMax);
    }

    //
    // The default min protocol version is set to TLS1.0 to avoid security issues with SSLv3
    //
    const string protocolVersionMin = properties->getPropertyWithDefault("IceSSL.ProtocolVersionMin", "tls1_0");
    if(!protocolVersionMin.empty())
    {
        _protocolVersionMin = parseProtocol(protocolVersionMin);
    }
    _initialized = true;
}

//
// Destroy the engine.
//
void
IceSSL::SecureTransportEngine::destroy()
{
}

SSLContextRef
IceSSL::SecureTransportEngine::newContext(bool incoming)
{
    SSLContextRef ssl = SSLCreateContext(kCFAllocatorDefault, incoming ? kSSLServerSide : kSSLClientSide,
                                         kSSLStreamType);
    if(!ssl)
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: unable to create SSL context");
    }

    OSStatus err = noErr;
    if(incoming)
    {
        switch(getVerifyPeer())
        {
            case 0:
            {
                SSLSetClientSideAuthenticate(ssl, kNeverAuthenticate);
                break;
            }
            case 1:
            {
                SSLSetClientSideAuthenticate(ssl, kTryAuthenticate);
                break;
            }
            case 2:
            {
                SSLSetClientSideAuthenticate(ssl, kAlwaysAuthenticate);
                break;
            }
            default:
            {
                assert(false);
                break;
            }
        }

        if(!_dhParams.empty())
        {
            if((err = SSLSetDiffieHellmanParams(ssl, &_dhParams[0], _dhParams.size())))
            {
                throw SecurityException(__FILE__, __LINE__,
                                        "IceSSL: unable to create the trust object:\n" + errorToString(err));
            }
        }
    }

    if(_chain && (err = SSLSetCertificate(ssl, _chain.get())))
    {
        throw SecurityException(__FILE__, __LINE__,
                                "IceSSL: error while setting the SSL context certificate:\n" + errorToString(err));
    }

    if(!_ciphers.empty())
    {
        if((err = SSLSetEnabledCiphers(ssl, &_ciphers[0], _ciphers.size())))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: error while setting ciphers:\n" + errorToString(err));
        }
    }

    if((err = SSLSetSessionOption(ssl, incoming ? kSSLSessionOptionBreakOnClientAuth :
                                                  kSSLSessionOptionBreakOnServerAuth,
                                  true)))
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: error while setting SSL option:\n" + errorToString(err));
    }

    if(_protocolVersionMax != kSSLProtocolUnknown)
    {
        if((err = SSLSetProtocolVersionMax(ssl, _protocolVersionMax)))
        {
            throw SecurityException(__FILE__, __LINE__,
                                    "IceSSL: error while setting SSL protocol version max:\n" + errorToString(err));
        }
    }

    if(_protocolVersionMin != kSSLProtocolUnknown)
    {
        if((err = SSLSetProtocolVersionMin(ssl, _protocolVersionMin)))
        {
            throw SecurityException(__FILE__, __LINE__,
                                    "IceSSL: error while setting SSL protocol version min:\n" + errorToString(err));
        }
    }

    return ssl;
}

CFArrayRef
IceSSL::SecureTransportEngine::getCertificateAuthorities() const
{
    return _certificateAuthorities.get();
}

string
IceSSL::SecureTransportEngine::getCipherName(SSLCipherSuite cipher) const
{
    return CiphersHelper::cipherName(cipher);
}

void
IceSSL::SecureTransportEngine::parseCiphers(const string& ciphers)
{
    vector<string> tokens;
    vector<CipherExpression> cipherExpressions;

    bool allCiphers = false;
    IceUtilInternal::splitString(ciphers, " \t", tokens);
    for(vector<string>::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
    {
        string token(*i);
        if(token == "ALL")
        {
            if(i != tokens.begin())
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: `ALL' must be first in cipher list `" + ciphers + "'");
            }
            allCiphers = true;
        }
        else if(token == "NONE")
        {
            if(i != tokens.begin())
            {
                throw PluginInitializationException(__FILE__, __LINE__,
                                                    "IceSSL: `NONE' must be first in cipher list `" + ciphers + "'");
            }
        }
        else
        {
            CipherExpression ce;
            if(token.find('!') == 0)
            {
                ce.negation = true;
                if(token.size() > 1)
                {
                    token = token.substr(1);
                }
                else
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: invalid cipher expression `" + token + "'");
                }
            }
            else
            {
                ce.negation = false;
            }

            if(token.find('(') == 0)
            {
                if(token.rfind(')') != token.size() - 1)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: invalid cipher expression `" + token + "'");
                }

                try
                {
                    ce.re = new RegExp(token.substr(1, token.size() - 2));
                }
                catch(const Ice::SyscallException&)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: invalid cipher expression `" + token + "'");
                }
            }
            else
            {
                ce.cipher = token;
            }

            cipherExpressions.push_back(ce);
        }
    }

    //
    // Context used to get the cipher list
    //
    SSLContextRef ctx = SSLCreateContext(kCFAllocatorDefault, kSSLServerSide, kSSLStreamType);
    size_t numSupportedCiphers = 0;
    SSLGetNumberSupportedCiphers(ctx, &numSupportedCiphers);

    vector<SSLCipherSuite> supported;
    supported.resize(numSupportedCiphers);

    OSStatus err = SSLGetSupportedCiphers(ctx, &supported[0], &numSupportedCiphers);
    CFRelease(ctx);
    if(err)
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: unable to get supported ciphers list:\n" + errorToString(err));
    }

    vector<SSLCipherSuite> enabled;
    if(allCiphers)
    {
        enabled = supported;
    }

    for(vector<CipherExpression>::const_iterator i = cipherExpressions.begin(); i != cipherExpressions.end(); ++i)
    {
        CipherExpression ce = *i;
        if(ce.negation)
        {
            for(vector<SSLCipherSuite>::iterator j = enabled.begin(); j != enabled.end();)
            {
                SSLCipherSuite cipher = *j;
                string name = CiphersHelper::cipherName(cipher);

                if(ce.cipher.empty())
                {
                    if(ce.re->match(name))
                    {
                        j = enabled.erase(j);
                        continue;
                    }
                }
                else
                {
                    if(ce.cipher == name)
                    {
                        j = enabled.erase(j);
                        continue;
                    }
                }
                j++;
            }
        }
        else
        {
            if(ce.cipher.empty())
            {
                for(vector<SSLCipherSuite>::const_iterator j = supported.begin(); j != supported.end(); ++j)
                {
                    SSLCipherSuite cipher = *j;
                    string name = CiphersHelper::cipherName(cipher);
                    if(ce.re->match(name))
                    {
                        vector<SSLCipherSuite>::const_iterator k = find(enabled.begin(), enabled.end(), cipher);
                        if(k == enabled.end())
                        {
                            enabled.push_back(cipher);
                        }
                    }
                }
            }
            else
            {
                SSLCipherSuite cipher = CiphersHelper::cipherForName(ce.cipher);
                vector<SSLCipherSuite>::const_iterator k = find(enabled.begin(), enabled.end(), cipher);
                if(k == enabled.end())
                {
                    enabled.push_back(cipher);
                }
            }
        }
    }
    _ciphers = enabled;

    if(_ciphers.empty())
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: invalid value for IceSSL.Ciphers:\n" + ciphers +
                                            "\nThe result cipher list does not contain any entries");
    }
}

SecKeychainRef
SecureTransportEngine::openKeychain()
{
    const PropertiesPtr properties = communicator()->getProperties();

    //
    // Open the application KeyChain or create it if the keychain doesn't exists
    //
    string keychainPath = properties->getProperty("IceSSL.Keychain");
    string keychainPassword = properties->getProperty("IceSSL.KeychainPassword");

    SecKeychainRef keychain = 0;
    OSStatus err = 0;
    if(keychainPath.empty())
    {
        if((err = SecKeychainCopyDefault(&keychain)))
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: unable to retrieve default keychain:\n" + errorToString(err));
        }
    }
    else
    {
        //
        // KeyChain path is relative to the current working directory.
        //
        if(!IceUtilInternal::isAbsolutePath(keychainPath))
        {
            string cwd;
            if(IceUtilInternal::getcwd(cwd) == 0)
            {
                keychainPath = string(cwd) + '/' + keychainPath;
            }
        }

        if((err = SecKeychainOpen(keychainPath.c_str(), &keychain)))
        {
            throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unable to open keychain: `" +
                                                keychainPath + "'\n" + errorToString(err));
        }
    }

    UniqueRef<SecKeychainRef> k(keychain);

    SecKeychainStatus status;
    err = SecKeychainGetStatus(keychain, &status);
    if(err == noErr)
    {
        const char* pass = keychainPassword.empty() ? 0 : keychainPassword.c_str();
        if((err = SecKeychainUnlock(keychain, keychainPassword.size(), pass, pass != 0)))
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: unable to unlock keychain:\n" + errorToString(err));
        }
    }
    else if(err == errSecNoSuchKeychain)
    {
        const char* pass = keychainPassword.empty() ? 0 : keychainPassword.c_str();
        if((err = SecKeychainCreate(keychainPath.c_str(), keychainPassword.size(), pass, pass == 0, 0, &keychain)))
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: unable to create keychain:\n" + errorToString(err));
        }
        k.reset(keychain);
    }
    else
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: unable to open keychain:\n" + errorToString(err));
    }

    //
    // Set keychain settings to avoid keychain lock.
    //
    SecKeychainSettings settings;
    settings.version = SEC_KEYCHAIN_SETTINGS_VERS1;
    settings.lockOnSleep = FALSE;
    settings.useLockInterval = FALSE;
    settings.lockInterval = INT_MAX;
    if((err = SecKeychainSetSettings(keychain, &settings)))
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: error setting keychain settings:\n" + errorToString(err));
    }

    return k.release();
}

#endif
