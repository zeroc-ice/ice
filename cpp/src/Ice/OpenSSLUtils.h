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

namespace IceSSL
{

namespace OpenSSL
{

std::string getGeneralizedTime(ASN1_GENERALIZEDTIME*);

std::string getUTCTime(ASN1_UTCTIME*);

std::string getASN1time(ASN1_TIME*);

DH* loadDHParam(const char*);

DH* getTempDH(unsigned char*, int, unsigned char*, int);

DH* getTempDH512();

std::string sslGetErrors();

}

}

extern "C"
{

RSA* tmpRSACallback(SSL*, int, int);

DH* tmpDHCallback(SSL*, int, int);

int verifyCallback(int, X509_STORE_CTX*);

int passwordCallback(char*, int, int, void*);

}
