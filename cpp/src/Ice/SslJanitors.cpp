// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslJanitors.h>

IceSecurity::Ssl::OpenSSL::RSAJanitor::RSAJanitor(RSA* rsa) :
                                      _rsa(rsa)
{
}

IceSecurity::Ssl::OpenSSL::RSAJanitor::~RSAJanitor()
{
    if (_rsa)
    {
        RSA_free(_rsa);
    }
}

void
IceSecurity::Ssl::OpenSSL::RSAJanitor::clear()
{
    _rsa = 0;
}

RSA*
IceSecurity::Ssl::OpenSSL::RSAJanitor::get() const
{
    return _rsa;
}


IceSecurity::Ssl::OpenSSL::EVP_PKEYJanitor::EVP_PKEYJanitor(EVP_PKEY* evp_pkey) :
                                           _evp_pkey(evp_pkey)
{
}

IceSecurity::Ssl::OpenSSL::EVP_PKEYJanitor::~EVP_PKEYJanitor()
{
    if (_evp_pkey)
    {
        EVP_PKEY_free(_evp_pkey);
    }
}

void
IceSecurity::Ssl::OpenSSL::EVP_PKEYJanitor::clear()
{
    _evp_pkey = 0;
}

EVP_PKEY*
IceSecurity::Ssl::OpenSSL::EVP_PKEYJanitor::get() const
{
    return _evp_pkey;
}

IceSecurity::Ssl::OpenSSL::X509_REQJanitor::X509_REQJanitor(X509_REQ* x509_req) :
                                           _x509_req(x509_req)
{
}

IceSecurity::Ssl::OpenSSL::X509_REQJanitor::~X509_REQJanitor()
{
    if (_x509_req)
    {
        X509_REQ_free(_x509_req);
    }
}

void
IceSecurity::Ssl::OpenSSL::X509_REQJanitor::clear()
{
    _x509_req = 0;
}

X509_REQ*
IceSecurity::Ssl::OpenSSL::X509_REQJanitor::get() const
{
    return _x509_req;
}



IceSecurity::Ssl::OpenSSL::X509Janitor::X509Janitor(X509* x509) :
                                       _x509(x509)
{
}

IceSecurity::Ssl::OpenSSL::X509Janitor::~X509Janitor()
{
    if (_x509)
    {
        X509_free(_x509);
    }
}

void
IceSecurity::Ssl::OpenSSL::X509Janitor::clear()
{
    _x509 = 0;
}

X509*
IceSecurity::Ssl::OpenSSL::X509Janitor::get() const
{
    return _x509;
}

