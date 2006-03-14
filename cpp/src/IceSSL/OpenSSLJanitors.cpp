// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/OpenSSLJanitors.h>

IceSSL::RSAJanitor::RSAJanitor(RSA* rsa) :
                            _rsa(rsa)
{
    assert(_rsa != 0);
}

IceSSL::RSAJanitor::~RSAJanitor()
{
    if(_rsa)
    {
        RSA_free(_rsa);
    }
}

void
IceSSL::RSAJanitor::clear()
{
    _rsa = 0;
}

RSA*
IceSSL::RSAJanitor::get() const
{
    return _rsa;
}

IceSSL::EVP_PKEYJanitor::EVP_PKEYJanitor(EVP_PKEY* evp_pkey) :
                                 _evp_pkey(evp_pkey)
{
    assert(_evp_pkey != 0);
}

IceSSL::EVP_PKEYJanitor::~EVP_PKEYJanitor()
{
    if(_evp_pkey)
    {
        EVP_PKEY_free(_evp_pkey);
    }
}

void
IceSSL::EVP_PKEYJanitor::clear()
{
    _evp_pkey = 0;
}

EVP_PKEY*
IceSSL::EVP_PKEYJanitor::get() const
{
    return _evp_pkey;
}

IceSSL::X509_REQJanitor::X509_REQJanitor(X509_REQ* x509_req) :
                                 _x509_req(x509_req)
{
    assert(_x509_req != 0);
}

IceSSL::X509_REQJanitor::~X509_REQJanitor()
{
    if(_x509_req)
    {
        X509_REQ_free(_x509_req);
    }
}

void
IceSSL::X509_REQJanitor::clear()
{
    _x509_req = 0;
}

X509_REQ*
IceSSL::X509_REQJanitor::get() const
{
    return _x509_req;
}

IceSSL::X509Janitor::X509Janitor(X509* x509) :
                             _x509(x509)
{
    assert(_x509 != 0);
}

IceSSL::X509Janitor::~X509Janitor()
{
    if(_x509)
    {
        X509_free(_x509);
    }
}

void
IceSSL::X509Janitor::clear()
{
    _x509 = 0;
}

X509*
IceSSL::X509Janitor::get() const
{
    return _x509;
}

IceSSL::BIOJanitor::BIOJanitor(BIO* bio) :
                            _bio(bio)
{
    assert(_bio != 0);
}

IceSSL::BIOJanitor::~BIOJanitor()
{
    if(_bio)
    {
        BIO_free(_bio);
    }
}

void
IceSSL::BIOJanitor::clear()
{
    _bio = 0;
}

BIO*
IceSSL::BIOJanitor::get() const
{
    return _bio;
}

