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

#include <Ice/SecurityException.h>

using Ice::SocketException;
using Ice::Exception;
using std::ostream;


void
IceSecurity::SecurityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    if (!_message.empty())
    {
        out << ":\n" << _message;
    }
}

void
IceSecurity::Ssl::ConfigParseException::ice_print(ostream& out) const
{
    SecurityException::ice_print(out);
}

void
IceSecurity::Ssl::ShutdownException::ice_print(ostream& out) const
{
    SecurityException::ice_print(out);
}

void
IceSecurity::Ssl::ProtocolException::ice_print(ostream& out) const
{
    SecurityException::ice_print(out);
}

void
IceSecurity::Ssl::CertificateException::ice_print(ostream& out) const
{
    SecurityException::ice_print(out);
}

void
IceSecurity::Ssl::OpenSSL::ContextException::ice_print(ostream& out) const
{
    SecurityException::ice_print(out);
}

