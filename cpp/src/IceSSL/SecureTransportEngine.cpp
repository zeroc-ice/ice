// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
parseProtocol(const string& prot)
{
    if(prot == "ssl2" || prot == "sslv2")
    {
        return kSSLProtocol2;
    }
    else if(prot == "ssl3" || prot == "sslv3")
    {
        return kSSLProtocol3;
    }
    else if(prot == "tls" || prot == "tls1" || prot == "tlsv1" || prot == "tls1_0" || prot == "tlsv1_0")
    {
        return kTLSProtocol1;
    }
    else if(prot == "tls1_1" || prot == "tlsv1_1")
    {
        return kTLSProtocol11;
    }
    else if(prot == "tls1_2" || prot == "tlsv1_2")
    {
        return kTLSProtocol12;
    }
    else
    {
        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unrecognized protocol `" + prot + "'");
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
    
    const string propPrefix = "IceSSL.";
    const PropertiesPtr properties = communicator()->getProperties();
    
    //
    // Check for a default directory. We look in this directory for
    // files mentioned in the configuration.
    //
    string defaultDir = properties->getProperty(propPrefix + "DefaultDir");
    
    //
    // Open the application KeyChain or create it if the keychain doesn't exists
    //
    string keychainPath = properties->getProperty("IceSSL.Keychain");
    string keychainPassword = properties->getProperty("IceSSL.KeychainPassword");
    
    bool usePassword = !keychainPassword.empty();
    size_t size = keychainPassword.size();
    const char* password = usePassword ? keychainPassword.c_str() : 0;
    
    CFDataRef hash = 0;
    SecKeychainRef keychain = 0;
    SecCertificateRef cert = 0;
    SecKeyRef key = 0;
    SecIdentityRef identity = 0;
    
    try
    {
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
        
            if((err = SecKeychainOpen(keychainPath.c_str(),  &keychain)))
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unable to open keychain: `" + 
                                                    keychainPath + "'\n" + errorToString(err));
            }
        }
        
        SecKeychainStatus status;
        err = SecKeychainGetStatus(keychain, &status);

        if(err == noErr)
        {
            if((err = SecKeychainUnlock(keychain, size, password, usePassword)))
            {
                throw PluginInitializationException(__FILE__, __LINE__, 
                                                    "IceSSL: unable to unlock keychain:\n" + errorToString(err));
            }
        }
        else if(err == errSecNoSuchKeychain)
        {
            if((err = SecKeychainCreate(keychainPath.c_str(), size, password, keychainPassword.empty(), 0, &keychain)))
            {
                throw PluginInitializationException(__FILE__, __LINE__, 
                                                    "IceSSL: unable to create keychain:\n" + errorToString(err));
            }
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
        
        int passwordRetryMax = properties->getPropertyAsIntWithDefault(propPrefix + "PasswordRetryMax", 3);
        PasswordPromptPtr passwordPrompt = getPasswordPrompt();
        
        //
        // Load the CA certificates used to authenticate peers into 
        // _certificateAuthorities array.
        //
        {
            try
            {
                string caFile = properties->getProperty(propPrefix + "CertAuthFile");
                if(!caFile.empty())
                {
                    if(!checkPath(caFile, defaultDir, false))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                            "IceSSL: CA certificate file not found:\n" + caFile);
                    }
                    _certificateAuthorities = loadCACertificates(caFile);
                }
            }
            catch(const CertificateReadException& ce)
            {
                throw PluginInitializationException(__FILE__, __LINE__, ce.reason);
            }
            catch(const CertificateEncodingException& ce)
            {
                throw PluginInitializationException(__FILE__, __LINE__, ce.reason);
            }
        }
        
        //
        // Import the application certificate and private keys into the application
        // keychain.
        //
        {
            string certFile = properties->getProperty(propPrefix + "CertFile");
            string keyFile = properties->getPropertyWithDefault(propPrefix + "KeyFile", certFile);
            vector<string>::size_type numCerts = 0;

            if(!certFile.empty())
            {
                vector<string> files;
                if(!IceUtilInternal::splitString(certFile, IceUtilInternal::pathsep, files) || files.size() > 2)
                {
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: invalid value for " + propPrefix +
                                                        "CertFile:\n" + certFile);
                }
                numCerts = files.size();
                for(vector<string>::iterator p = files.begin(); p != files.end();)
                {
                    string file = *p;
                    if(!checkPath(file, defaultDir, false))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                            "IceSSL: certificate file not found:\n" + file);
                    }
                    
                    try
                    {
                        loadCertificate(&cert, &hash, &key, keychain, file,
                                        properties->getProperty(propPrefix + "Password"), passwordPrompt, 
                                        passwordRetryMax);
                        break;
                    }
                    catch(const CertificateReadException& ce)
                    {
                        //
                        // If this is the last certificate rethrow the exception as PluginInitializationException,
                        // otherwise try the next certificate.
                        //
                        if(++p == files.end())
                        {
                            throw PluginInitializationException(__FILE__, __LINE__, ce.reason);
                        }
                    }
                }
            }
            
            if(!key && !keyFile.empty())
            {
                vector<string> files;
                if(!IceUtilInternal::splitString(keyFile, IceUtilInternal::pathsep, files) || files.size() > 2)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: invalid value for " + propPrefix + "KeyFile:\n" + keyFile);
                }
                if(files.size() != numCerts)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                        "IceSSL: IceSSL.KeyFile does not agree with IceSSL.CertFile");
                }
                for(vector<string>::iterator p = files.begin(); p != files.end();)
                {
                    string file = *p;
                    if(!checkPath(file, defaultDir, false))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                            "IceSSL: key file not found:\n" + file);
                    }

                    try
                    {
                        loadPrivateKey(&key, keyLabel(cert), hash, keychain, file, 
                                       properties->getProperty(propPrefix + "Password"), 
                                       passwordPrompt, passwordRetryMax);
                        break;
                    }
                    catch(const CertificateReadException& ce)
                    {
                        //
                        // If this is the last key rethrow the exception as PluginInitializationException,
                        // otherwise try the next certificate.
                        //
                        if(++p == files.end())
                        {
                            PluginInitializationException ex(__FILE__, __LINE__, ce.reason);
                            throw ex;
                        }
                    }
                }
            }

            if(!cert)
            {
                const string prop = propPrefix + "SecureTransport.FindCert";
                string val = properties->getProperty(prop);
                if(!val.empty())
                {
                    if(!(cert = findCertificates(keychain, prop, val)))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: no certificates found");
                    }
                }
            }
            
            if(cert)
            {
                if((err = SecIdentityCreateWithCertificate(keychain, cert, &identity)) != noErr)
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: error creating certificate identity:\n" + errorToString(err));
                }
            }
            
            if(identity)
            {
                SecTrustRef trust = 0;
                try
                {
                    //
                    // Retrieve the certificate chain
                    //
                    SecPolicyRef policy = SecPolicyCreateSSL(true, 0);
                    err = SecTrustCreateWithCertificates((CFArrayRef)cert, policy, &trust);
                    CFRelease(policy);
                    if(err || !trust)
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                    "IceSSL: error creating trust object" + (err ? ":\n" + errorToString(err) : ""));
                    }
                    
                    if((err = SecTrustSetAnchorCertificates(trust, _certificateAuthorities)))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                    "IceSSL: error while establish the anchor certificates:\n" + errorToString(err));
                    }
                    
                    SecTrustResultType trustResult;
                    if((err = SecTrustEvaluate(trust, &trustResult)))
                    {
                        throw PluginInitializationException(__FILE__, __LINE__, 
                                                            "IceSSL: error evaluating trust:\n" + errorToString(err));
                    }
                    
                    int chainLength = SecTrustGetCertificateCount(trust);
                    _chain = CFArrayCreateMutable(kCFAllocatorDefault, chainLength, &kCFTypeArrayCallBacks);
                    CFArrayAppendValue(_chain, identity);
                    for(int i = 1; i < chainLength; ++i)
                    {
                        CFArrayAppendValue(_chain, SecTrustGetCertificateAtIndex(trust, i));
                    }
                    CFRelease(trust);
                }
                catch(...)
                {
                    if(trust)
                    {
                        CFRelease(trust);
                    }
                    throw;
                }
            }
            
            if(hash)
            {
                CFRelease(hash);
            }
            
            if(keychain)
            {
                CFRelease(keychain);
            }
            
            if(cert)
            {
                CFRelease(cert);
            }
            
            if(key)
            {
                CFRelease(key);
            }
            
            if(identity)
            {
                CFRelease(identity);
            }
        }
    }
    catch(...)
    {    
        if(hash)
        {
            CFRelease(hash);
        }
        
        if(keychain)
        {
            CFRelease(keychain);
        }
        
        if(cert)
        {
            CFRelease(cert);
        }
        
        if(key)
        {
            CFRelease(key);
        }
        
        if(identity)
        {
            CFRelease(identity);
        }
        
        throw;
    }
    //
    // DiffieHellmanParams in DER format.
    //
    string dhFile = properties->getProperty(propPrefix + "DHParams");
    if(!dhFile.empty())
    {
        if(!checkPath(dhFile, defaultDir, false))
        {
            throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: DH params file not found:\n" + dhFile);
        }
        
        readFile(dhFile, _dhParams);
    }
    
    //
    // Establish the cipher list.
    //
    string ciphers = properties->getProperty(propPrefix + "Ciphers");
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
    const string protocolVersionMax = properties->getProperty(propPrefix + "ProtocolVersionMax");
    if(!protocolVersionMax.empty())
    {
        _protocolVersionMax = parseProtocol(protocolVersionMax);
    }
    
    const string protocolVersionMin = properties->getProperty(propPrefix + "ProtocolVersionMin");
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
    if(_certificateAuthorities)
    {
        CFRelease(_certificateAuthorities);
        _certificateAuthorities = 0;
    }
    
    if(_chain)
    {
        CFRelease(_chain);
        _chain = 0;
    }
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
    
    if(_chain && (err = SSLSetCertificate(ssl, _chain)))
    {        
        throw SecurityException(__FILE__, __LINE__, 
                                "IceSSL: error while set the SSL context certificate:\n" + errorToString(err));
    }
    

    if(!_ciphers.empty())
    {
        if((err = SSLSetEnabledCiphers(ssl, &_ciphers[0], _ciphers.size())))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: error while set ciphers:\n" + errorToString(err));
        }
    }
    
    if((err = SSLSetSessionOption(ssl, incoming ? kSSLSessionOptionBreakOnClientAuth : 
                                                  kSSLSessionOptionBreakOnServerAuth,
                                  true)))
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: error while set SSL option:\n" + errorToString(err));
    }
    
    if(_protocolVersionMax != kSSLProtocolUnknown)
    {
        if((err = SSLSetProtocolVersionMax(ssl, _protocolVersionMax)))
        {
            throw SecurityException(__FILE__, __LINE__, 
                                    "IceSSL: error while set SSL protocol version max:\n" + errorToString(err));
        }
    }
    
    if(_protocolVersionMin != kSSLProtocolUnknown)
    {
        if((err = SSLSetProtocolVersionMin(ssl, _protocolVersionMin)))
        {
            throw SecurityException(__FILE__, __LINE__, 
                                    "IceSSL: error while set SSL protocol version min:\n" + errorToString(err));
        }
    }
    
    return ssl;
}

CFArrayRef
IceSSL::SecureTransportEngine::getCertificateAuthorities() const
{
    return _certificateAuthorities;
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
}
#endif
