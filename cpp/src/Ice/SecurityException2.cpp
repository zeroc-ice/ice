// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Note: This pragma is used to disable spurious warning messages having
//       to do with the length of debug symbols exceeding 255 characters.
//       This is due to STL template identifiers expansion.
//       The MSDN Library recommends that you put this pragma directive
//       in place to avoid the warnings.
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

