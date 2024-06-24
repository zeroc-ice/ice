//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/SSL/SSLException.h"

#include <string>

using namespace std;
using namespace Ice::SSL;

CertificateReadException::CertificateReadException(const char* file, int line, string r) noexcept
    : Exception(file, line),
      reason(std::move(r))
{
}

const char*
CertificateReadException::ice_id() const
{
    return "::Ice::SSL::CertificateReadException";
}

CertificateEncodingException::CertificateEncodingException(const char* file, int line, string r) noexcept
    : Exception(file, line),
      reason(std::move(r))
{
}

const char*
CertificateEncodingException::ice_id() const
{
    return "::Ice::SSL::CertificateEncodingException";
}

ParseException::ParseException(const char* file, int line, string r) noexcept
    : Exception(file, line),
      reason(std::move(r))
{
}

const char*
ParseException::ice_id() const
{
    return "::Ice::SSL::ParseException";
}
