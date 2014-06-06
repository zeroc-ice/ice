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
#include <dirent.h>

using namespace std;
using namespace IceUtil;
using namespace Ice;
using namespace IceSSL;

namespace
{

vector<string>
dir(const string& path)
{
    vector<string> result;
    
    DIR* d = opendir(path.c_str());
    if(!d)
    {
        ostringstream os;
        os << "failed to open dir `" << path << "'";
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, os.str());
    }
    
    struct dirent* dp = 0;
    while((dp = readdir(d)))
    {
        string name(dp->d_name);
        if(IceUtilInternal::fileExists(path + "/" + name))
        {
            result.push_back(name);
        }
    }
    closedir(d);
    return result;
}

class RegExp : public IceUtil::Shared
{
public:
    
    RegExp(const std::string&);
    ~RegExp();
    bool match(const std::string&);
    
private:
    
    regex_t _preg;
};
typedef IceUtil::Handle<RegExp> RegExpPtr;

RegExp::RegExp(const std::string& regexp)
{
    int err = regcomp(&_preg, regexp.c_str(), REG_EXTENDED | REG_NOSUB);
    if(err != 0)
    {
        ostringstream os;
        os << "failed to compiler regular expression `" << regexp << "' (error = " << err << ")";
        throw IllegalArgumentException(__FILE__, __LINE__, os.str());
    }
}

RegExp::~RegExp()
{
    regfree(&_preg);
}

bool
RegExp::match(const std::string& value)
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

SSLCipherSuite
CiphersHelper::cipherForName(const string& name)
{
    map<string, SSLCipherSuite>::const_iterator i = _ciphers.find(name);
    if(i == _ciphers.end() || i->second == SSL_NO_SUCH_CIPHERSUITE)
    {
        PluginInitializationException ex(__FILE__, __LINE__, "IceSSL: no such cipher " + name);
        throw ex;
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

SSLProtocol
parseProtocol(const std::string& prot)
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
        PluginInitializationException ex(__FILE__, __LINE__);
        ex.reason = "IceSSL: unrecognized protocol `" + prot + "'";
        throw ex;
    }
}

}

IceUtil::Shared* IceSSL::upCast(IceSSL::SecureTransportEngine* p) { return p; }

IceSSL::SecureTransportEngine::SecureTransportEngine(const Ice::CommunicatorPtr& communicator) :
    SSLEngine(communicator),
    _initialized(false),
    _ctx(0),
    _certificateAuthorities(0),
    _cert(0),
    _key(0),
    _identity(0),
    _keychain(0),
    _protocolVersionMax(kSSLProtocolUnknown),
    _protocolVersionMin(kSSLProtocolUnknown),
    _dhParams(0),
    _dhParamsLength(0),
    _ciphers(new ScopedArray<SSLCipherSuite>()),
    _allCiphers(false),
    _numCiphers(-1)
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
    
    //
    // KeyChain path is relative to the current working directory.
    //
    if(keychainPath.empty())
    {
        keychainPath = "login.keychain";
    }
    else
    {
        if(!IceUtilInternal::isAbsolutePath(keychainPath))
        {
            string cwd;
            if(IceUtilInternal::getcwd(cwd) == 0)
            {
                keychainPath = string(cwd) + '/' + keychainPath;
            }
        }
    }
    
    bool usePassword = !keychainPassword.empty();
    size_t size = keychainPassword.size();
    const char* password = usePassword ? keychainPassword.c_str() : 0;
    
    OSStatus err = SecKeychainOpen(keychainPath.c_str(),  &_keychain);
    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: unable to open keychain: `" << keychainPath << "'\n" << errorToString(err);
        throw PluginInitializationException(__FILE__, __LINE__, os.str());
    }
    
    SecKeychainStatus status;
    err = SecKeychainGetStatus(_keychain, &status);

    if(err == noErr)
    {
        err = SecKeychainUnlock(_keychain, size, password, usePassword);
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: unable to unlock keychain: `" << keychainPath << "'\n" << errorToString(err);
            throw PluginInitializationException(__FILE__, __LINE__, os.str());
        }
    }
    else if(err == errSecNoSuchKeychain)
    {
        err = SecKeychainCreate(keychainPath.c_str(), size, password, keychainPassword.empty(), 0, &_keychain);
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: unable to create keychain: `" << keychainPath << "'\n" << errorToString(err);
            throw PluginInitializationException(__FILE__, __LINE__, os.str());
        }
    }
    else
    {
        ostringstream os;
        os << "IceSSL: unable to open keychain: `" << keychainPath << "'\n" << errorToString(err);
        throw PluginInitializationException(__FILE__, __LINE__, os.str());
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
                    PluginInitializationException ex(__FILE__, __LINE__);
                    ex.reason = "IceSSL: CA certificate file not found:\n" + caFile;
                    throw ex;
                }
                _certificateAuthorities = loadCACertificates(caFile);
            }
        }
        catch(const CertificateReadException& ce)
        {
            PluginInitializationException ex(__FILE__, __LINE__, ce.reason);
            throw ex;
        }
        
        string caDir = properties->getPropertyWithDefault(propPrefix + "CertAuthDir", defaultDir);
        if(!caDir.empty())
        {
            CFMutableArrayRef certificateAuthorities;
            if(_certificateAuthorities)
            {
                certificateAuthorities = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, _certificateAuthorities);
                CFRelease(_certificateAuthorities);
            }
            else
            {
                certificateAuthorities = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
            }
            
            vector<string> files = dir(caDir);
            for(vector<string>::const_iterator i = files.begin(); i != files.end(); ++i)
            {
                try
                {
                    CFArrayRef tmp = loadCACertificates(caDir + "/" + *i);
                    
                    CFArrayAppendArray(certificateAuthorities, tmp, CFRangeMake(0, CFArrayGetCount(tmp)));
                    CFRelease(tmp);
                }
                catch(const CertificateReadException&)
                {
                    //
                    // Some files in CertAuthDir might not be certificates, we just ignore those files.
                    //
                }
            }
            _certificateAuthorities = certificateAuthorities;
        }
    }
    
    //
    // Import the application certificate and private keys into the application
    // keychain.
    //
    {
        string certFile = properties->getProperty(propPrefix + "CertFile");
        string keyFile = properties->getProperty(propPrefix + "KeyFile");
        vector<string>::size_type numCerts = 0;
        
        CFDataRef hash = 0;

        if(!certFile.empty())
        {
            try
            {
                vector<string> files;
                if(!IceUtilInternal::splitString(certFile, IceUtilInternal::pathsep, files) || files.size() > 2)
                {
                    PluginInitializationException ex(__FILE__, __LINE__);
                    ex.reason = "IceSSL: invalid value for " + propPrefix + "CertFile:\n" + certFile;
                    throw ex;
                }
                numCerts = files.size();
                for(vector<string>::iterator p = files.begin(); p != files.end(); ++p)
                {
                    string file = *p;
                    if(!checkPath(file, defaultDir, false))
                    {
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = "IceSSL: certificate file not found:\n" + file;
                        throw ex;
                    }
                    
                    loadCertificate(&_cert, &hash, keyFile.empty() ? &_key : 0, _keychain, file,
                                    properties->getProperty(propPrefix + "Password"), passwordPrompt, 
                                    passwordRetryMax);
                    break;
                }
            }
            catch(const CertificateReadException& ce)
            {
                PluginInitializationException ex(__FILE__, __LINE__, ce.reason);
                throw ex;
            }
        }
        
        if(!keyFile.empty())
        {
            try
            {
                vector<string> files;
                if(!IceUtilInternal::splitString(keyFile, IceUtilInternal::pathsep, files) || files.size() > 2)
                {
                    PluginInitializationException ex(__FILE__, __LINE__);
                    ex.reason = "IceSSL: invalid value for " + propPrefix + "KeyFile:\n" + keyFile;
                    throw ex;
                }
                if(files.size() != numCerts)
                {
                    PluginInitializationException ex(__FILE__, __LINE__);
                    ex.reason = "IceSSL: " + propPrefix + "KeyFile does not agree with " + propPrefix + "CertFile";
                    throw ex;
                }
                for(vector<string>::iterator p = files.begin(); p != files.end(); ++p)
                {
                    string file = *p;
                    if(!checkPath(file, defaultDir, false))
                    {
                        PluginInitializationException ex(__FILE__, __LINE__);
                        ex.reason = "IceSSL: key file not found:\n" + file;
                        throw ex;
                    }
                    //
                    // The private key may be stored in an encrypted file, so handle
                    // password retries.
                    //
                    loadPrivateKey(&_key, keyLabel(_cert), hash, _keychain, file, 
                                    properties->getProperty(propPrefix + "Password"), 
                                    passwordPrompt, passwordRetryMax);
                    break;
                }
            }
            catch(const CertificateReadException& ce)
            {
                PluginInitializationException ex(__FILE__, __LINE__, ce.reason);
                throw ex;
            }
        }
        
        if(_cert)
        {
            err = SecIdentityCreateWithCertificate(_keychain, _cert, &_identity);
            if(err != noErr)
            {
                ostringstream os;
                os << "IceSSL: unable to create the certificate identity:\n" << errorToString(err);
                PluginInitializationException ex(__FILE__, __LINE__, os.str());
                throw ex;
            }
        }
        
        if(hash)
        {
            CFRelease(hash);
        }
    }
    
    //
    // DiffieHellmanParams in DER format.
    //
    {
        string dhParams = properties->getProperty(propPrefix + "DHParams");
        if(!dhParams.empty())
        {
            if(!checkPath(dhParams, defaultDir, false))
            {
                PluginInitializationException ex(__FILE__, __LINE__);
                ex.reason = "IceSSL: DH params file not found:\n" + dhParams;
                throw ex;
            }
            
            ScopedArray<char> buffer;
            _dhParamsLength = readFile(dhParams, buffer);
            _dhParams.reset(new ScopedArray<char>(buffer));
        }
    }
    
    //
    // Establish the cipher list.
    //
    string ciphers = properties->getProperty(propPrefix + "Ciphers");
    if(!ciphers.empty())
    {
        //
        // Context used to get the cipher list
        //
        _ctx = SSLCreateContext(kCFAllocatorDefault, kSSLServerSide, kSSLStreamType);
        CiphersHelper::initialize();
        parseCiphers(ciphers);
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
    
    if(_identity)
    {
        CFRelease(_identity);
        _identity = 0;
    }
    
    if(_cert)
    {
        CFRelease(_cert);
        _cert = 0;
    }
    
    if(_key)
    {
        CFRelease(_key);
        _key = 0;
    }
    
    if(_keychain)
    {
        CFRelease(_keychain);
        _keychain = 0;
    }
    
    if(_ctx)
    {
        CFRelease(_ctx);
        _ctx = 0;
    }
}

ContextRef
IceSSL::SecureTransportEngine::newContext(bool incoming)
{
    ContextRef ssl = SSLCreateContext(kCFAllocatorDefault, incoming ? kSSLServerSide : kSSLClientSide, kSSLStreamType);
    if(!ssl)
    {
        PluginInitializationException ex(__FILE__, __LINE__, "IceSSL: unable to create SSL context");
        throw ex;
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
        
        if(_dhParamsLength > 0)
        {
            err = SSLSetDiffieHellmanParams(ssl, _dhParams->get(), _dhParamsLength);
            if(err != noErr)
            {
                ostringstream os;
                os << "IceSSL: unable to create the trust object:\n" << errorToString(err);
                PluginInitializationException ex(__FILE__, __LINE__, os.str());
                throw ex;
            }
        }
    }
    
    if(_cert)
    {
        //
        // Retrieve the certificate chain
        //
        SecPolicyRef policy = SecPolicyCreateSSL(true, 0);
        SecTrustRef trust;
        err = SecTrustCreateWithCertificates((CFArrayRef)_cert, policy, &trust);
        if(err != noErr || !trust)
        {
            ostringstream os;
            os << "IceSSL: unable to create the trust object";
            if(err != noErr)
            {
                os << '\n' << errorToString(err);
            }
            PluginInitializationException ex(__FILE__, __LINE__, os.str());
            throw ex;
        }
        
        err = SecTrustSetAnchorCertificates(trust, _certificateAuthorities);
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: unable to establish the anchor certificates\n" << errorToString(err);
            PluginInitializationException ex(__FILE__, __LINE__, os.str());
            throw ex;
        }
        
        SecTrustResultType trustResult;
        err = SecTrustEvaluate(trust, &trustResult);
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: unable to evaluate trust:\n" << errorToString(err);
            PluginInitializationException ex(__FILE__, __LINE__, os.str());
            throw ex;
        }
        
        int chainLength = SecTrustGetCertificateCount(trust);
        CFMutableArrayRef chain = CFArrayCreateMutable(kCFAllocatorDefault, chainLength, &kCFTypeArrayCallBacks);
        CFArrayAppendValue(chain, _identity);
        for(int i = 1; i < chainLength; ++i)
        {
            CFArrayAppendValue(chain, SecTrustGetCertificateAtIndex(trust, i));
        }
        CFRelease(trust);
        
        err = SSLSetCertificate(ssl, chain);
        
        CFRelease(chain);
        
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: unable to set the SSL context certificate identity:\n" << errorToString(err);
            PluginInitializationException ex(__FILE__, __LINE__, os.str());
            throw ex;
        }
    }
    

    if(_numCiphers != -1)
    {
        err = SSLSetEnabledCiphers(ssl, _ciphers->get(), _numCiphers);
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: failed to set enabled ciphers:\n" << errorToString(err);
            PluginInitializationException ex(__FILE__, __LINE__, os.str());
            throw ex;
        }
    }
    err = SSLSetSessionOption(ssl, incoming ? kSSLSessionOptionBreakOnClientAuth : kSSLSessionOptionBreakOnServerAuth,
                              true);
    
    if(err != noErr)
    {
        ostringstream os;
        os << "IceSSL: failed to set SSL option:\n" << errorToString(err);
        PluginInitializationException ex(__FILE__, __LINE__, os.str());
        throw ex;
    }
    
    if(_protocolVersionMax != kSSLProtocolUnknown)
    {
        err = SSLSetProtocolVersionMax(ssl, _protocolVersionMax);
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: failed to set SSL protocol version max:\n" << errorToString(err);
            PluginInitializationException ex(__FILE__, __LINE__, os.str());
            throw ex;
        }
    }
    
    if(_protocolVersionMin != kSSLProtocolUnknown)
    {
        err = SSLSetProtocolVersionMin(ssl, _protocolVersionMin);
        if(err != noErr)
        {
            ostringstream os;
            os << "IceSSL: failed to set SSL protocol version min:\n" << errorToString(err);
            PluginInitializationException ex(__FILE__, __LINE__, os.str());
            throw ex;
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
    
    IceUtilInternal::splitString(ciphers, " \t", tokens);
    for(vector<string>::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
    {
        string token(*i);
        if(token == "ALL")
        {
            if(i != tokens.begin())
            {
                ostringstream os;
                os << "IceSSL: `ALL' must be first in cipher list `" << ciphers << "'";
                PluginInitializationException ex(__FILE__, __LINE__, os.str());
                throw ex;
            }
            _allCiphers = true;
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
                    ostringstream os;
                    os << "IceSSL: invalid cipher expression `" << token << "'";
                    PluginInitializationException ex(__FILE__, __LINE__, os.str());
                    throw ex;
                }
            }
            
            if(token.find('(') == 0)
            {
                if(token.rfind(')') != token.size() - 1)
                {
                    ostringstream os;
                    os << "IceSSL: invalid cipher expression `" << token << "'";
                    PluginInitializationException ex(__FILE__, __LINE__, os.str());
                    throw ex;
                }

                try
                {
                    ce.re = new RegExp(token.substr(1, token.size() - 2));
                }
                catch(const Ice::SyscallException&)
                {
                    ostringstream os;
                    os << "IceSSL: invalid cipher expression `" << token << "'";
                    PluginInitializationException ex(__FILE__, __LINE__, os.str());
                    throw ex;
                }
            }
            else
            {
                ce.cipher = token;
            }
            
            cipherExpressions.push_back(ce);
        }
    }
    size_t numSupportedCiphers = 0;
    SSLGetNumberSupportedCiphers(_ctx, &numSupportedCiphers);
    
    ScopedArray<SSLCipherSuite> buffer(new SSLCipherSuite[numSupportedCiphers]);
    
    OSStatus err;
    if((err = SSLGetSupportedCiphers(_ctx, buffer.get(), &numSupportedCiphers)) != noErr)
    {
        ostringstream os;
        os << "IceSSL: unable to get supported ciphers list (error = " << err << ")";
        PluginInitializationException ex(__FILE__, __LINE__, os.str());
        throw ex;
    }
    
    SSLCipherSuite* supported = buffer.get();
    vector<SSLCipherSuite> allCiphers;
    if(_allCiphers)
    {
        for(int i = 0; i < numSupportedCiphers; ++i)
        {
            allCiphers.push_back(supported[i]);
        }
    }
    
    for(vector<CipherExpression>::const_iterator i = cipherExpressions.begin(); i != cipherExpressions.end(); ++i)
    {
        CipherExpression ce = *i;
        if(ce.negation)
        {
            for(vector<SSLCipherSuite>::iterator j = allCiphers.begin(); j != allCiphers.end();)
            {
                SSLCipherSuite cipher = *j;
                string name = CiphersHelper::cipherName(cipher);
                
                if(ce.cipher.empty())
                {
                    if(ce.re->match(name))
                    {
                        j = allCiphers.erase(j);
                        continue;
                    }
                }
                else
                {
                    if(ce.cipher == name)
                    {
                        j = allCiphers.erase(j);
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
                for(int i = 0; i < numSupportedCiphers; ++i)
                {
                    SSLCipherSuite cipher = supported[i];
                    string name = CiphersHelper::cipherName(cipher);
                    if(ce.re->match(name))
                    {
                        vector<SSLCipherSuite>::const_iterator k = find(allCiphers.begin(), allCiphers.end(), cipher);
                        if(k == allCiphers.end())
                        {
                            allCiphers.push_back(cipher);
                        }
                    }
                }
            }
            else
            {
                SSLCipherSuite cipher = CiphersHelper::cipherForName(ce.cipher);
                vector<SSLCipherSuite>::const_iterator k = find(allCiphers.begin(), allCiphers.end(), cipher);
                if(k == allCiphers.end())
                {
                    allCiphers.push_back(cipher);
                }
            }
        }
    }
    
    if(!allCiphers.empty())
    {
        _ciphers.reset(new ScopedArray<SSLCipherSuite>(new SSLCipherSuite[allCiphers.size()]));
        SSLCipherSuite* enabled = _ciphers->get();
        for(vector<SSLCipherSuite>::const_iterator i = allCiphers.begin(); i != allCiphers.end(); ++i)
        {
            *(enabled++) = *i;
        }
    }
    _numCiphers = allCiphers.size();
}

SecCertificateRef
IceSSL::SecureTransportEngine::getCertificate() const
{
    return _cert;
}

SecKeychainRef
IceSSL::SecureTransportEngine::getKeychain() const
{
    return _keychain;
}

#endif
