// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <string>
#include <algorithm>

#include <Ice/SslGeneralConfig.h>

using namespace std;

IceSecurity::Ssl::GeneralConfig::GeneralConfig()
{
    _sslVersion = SSL_V23;

    _verifyMode = SSL_VERIFY_NONE;
    _verifyDepth = 10;

    _context = "";
    _cipherList = "";
    _randomBytesFiles = "";
}

void
IceSecurity::Ssl::GeneralConfig::set(string& name, string& value)
{
    if (name.compare("version") == 0)
    {
        parseVersion(value);
    }
    else if (name.compare("cipherlist") == 0)
    {
        _cipherList = value;
    }
    else if (name.compare("context") == 0)
    {
        _context = value;
    }
    else if (name.compare("verifymode") == 0)
    {
        parseVerifyMode(value);
    }
    else if (name.compare("verifydepth") == 0)
    {
        _verifyDepth = atoi(value.c_str());
    }
    else if (name.compare("randombytes") == 0)
    {
        _randomBytesFiles = value;
    }
    return;
}

//
// Protected Methods
//

void
IceSecurity::Ssl::GeneralConfig::parseVersion(string& value)
{
    if (value.compare("SSLv2") == 0)
    {
        _sslVersion = SSL_V2;
    }
    else if (value.compare("SSLv23") == 0)
    {
        _sslVersion = SSL_V23;
    }
    else if (value.compare("SSLv3") == 0)
    {
        _sslVersion = SSL_V3;
    }
    else if (value.compare("TLSv1") == 0)
    {
        _sslVersion = TLS_V1;
    }

    return;
}

void
IceSecurity::Ssl::GeneralConfig::parseVerifyMode(string& value)
{
    const string delim = " |\t\n\r";

    string s(value);
    transform(s.begin(), s.end(), s.begin(), tolower);

    string::size_type beg;
    string::size_type end = 0;

    while (true)
    {
	beg = s.find_first_not_of(delim, end);

        if (beg == string::npos)
	{
	    break;
	}
	
	end = s.find_first_of(delim, beg);

        if (end == string::npos)
	{
	    end = s.length();
	}

        string option = s.substr(beg, end - beg);

        if (option.compare("none") == 0)
        {
            _verifyMode |= SSL_VERIFY_NONE;
        }
        else if (option.compare("peer") == 0)
        {
            _verifyMode |= SSL_VERIFY_PEER;
        }
        else if (option.compare("fail_no_cert") == 0)
        {
            _verifyMode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
        }
        else if (option.compare("client_once") == 0)
        {
            _verifyMode |= SSL_VERIFY_CLIENT_ONCE;
        }
    }

    // Both SSL_VERIFY_FAIL_IF_NO_PEER_CERT and SSL_VERIFY_CLIENT_ONCE require
    // that SSL_VERIFY_PEER be set, otherwise it's an error.
    if ((_verifyMode != SSL_VERIFY_NONE) && !(_verifyMode & SSL_VERIFY_PEER))
    {
        _verifyMode = SSL_VERIFY_NONE;
    }

    return;
}
