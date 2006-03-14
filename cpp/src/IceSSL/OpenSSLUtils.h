// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Config.h>

#include <openssl/ssl.h>

namespace IceSSL
{

std::string getGeneralizedTime(ASN1_GENERALIZEDTIME*);

std::string getUTCTime(ASN1_UTCTIME*);

std::string getASN1time(ASN1_TIME*);

DH* loadDHParam(const char*);

DH* getTempDH(unsigned char*, int, unsigned char*, int);

DH* getTempDH512();
DH* getTempDH1024();
DH* getTempDH2048();
DH* getTempDH4096();

std::string sslGetErrors();

std::string getVerificationError(long);

}

extern "C"
{

RSA* tmpRSACallback(SSL*, int, int);

DH* tmpDHCallback(SSL*, int, int);

int verifyCallback(int, X509_STORE_CTX*);

int passwordCallback(char*, int, int, void*);

}
