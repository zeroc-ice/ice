// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <openssl/ssl.h>
#include <string>

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

using std::string;

string getGeneralizedTime(ASN1_GENERALIZEDTIME *tm);

string getUTCTime(ASN1_UTCTIME *tm);

string getASN1time(ASN1_TIME *tm);

}

}

}
