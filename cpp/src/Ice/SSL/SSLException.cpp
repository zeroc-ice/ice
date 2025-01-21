// Copyright (c) ZeroC, Inc.

#include "Ice/SSL/SSLException.h"

using namespace std;
using namespace Ice::SSL;

const char*
CertificateReadException::ice_id() const noexcept
{
    return "::Ice::SSL::CertificateReadException";
}

const char*
CertificateEncodingException::ice_id() const noexcept
{
    return "::Ice::SSL::CertificateEncodingException";
}
