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

#include <Ice/SslException.h>

using Ice::SocketException;
using Ice::Exception;

using std::ostream;
using std::string;

//////////////////////////////////
////////// SecurityException /////
//////////////////////////////////

//
// Public Methods
//

IceSecurity::SecurityException::SecurityException(const char* errMessage, const char* file, int line) :
                               LocalException(file, line),
                               _message(errMessage)
{
}

IceSecurity::SecurityException::SecurityException(const SecurityException& ex) :
                               LocalException(ex)
{
}

IceSecurity::SecurityException&
IceSecurity::SecurityException::operator=(const SecurityException& ex)
{
    LocalException::operator=(ex);
    return *this;
}

string
IceSecurity::SecurityException::toString() const
{
    string s = ice_name();

    if (_message == "")
    {
        s += ": unspecified exception";
    }
    else
    {
	s += ": ";
        s += _message;
    }

    return s;
}

string
IceSecurity::SecurityException::ice_name() const
{
    return "IceSecurity::SecurityException";
}

void
IceSecurity::SecurityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown security exception";
}

Ice::Exception*
IceSecurity::SecurityException::ice_clone() const
{
    return new SecurityException(*this);
}

void
IceSecurity::SecurityException::ice_throw() const
{
    throw *this;
}

///////////////////////////////////////
//////// ConfigParseException /////////
///////////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::ConfigParseException::ConfigParseException(const char* errMessage, const char* file, int line) :
                                       SecurityException(errMessage, file, line)
{
}

IceSecurity::Ssl::ConfigParseException::ConfigParseException(const ConfigParseException& ex) :
                                       SecurityException(ex)
{
}

IceSecurity::Ssl::ConfigParseException&
IceSecurity::Ssl::ConfigParseException::operator=(const ConfigParseException& ex)
{
    SecurityException::operator=(ex);
    return *this;
}

string
IceSecurity::Ssl::ConfigParseException::ice_name() const
{
    return "IceSecurity::ConfigParseException";
}

void
IceSecurity::Ssl::ConfigParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity system shutdown exception";
}

Ice::Exception*
IceSecurity::Ssl::ConfigParseException::ice_clone() const
{
    return new ConfigParseException(*this);
}

void
IceSecurity::Ssl::ConfigParseException::ice_throw() const
{
    throw *this;
}

///////////////////////////////////////
////////// ShutdownException //////////
///////////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::ShutdownException::ShutdownException(const char* errMessage, const char* file, int line) :
                                    SecurityException(errMessage, file, line)
{
}

IceSecurity::Ssl::ShutdownException::ShutdownException(const ShutdownException& ex) :
                                    SecurityException(ex)
{
}

IceSecurity::Ssl::ShutdownException&
IceSecurity::Ssl::ShutdownException::operator=(const ShutdownException& ex)
{
    SecurityException::operator=(ex);
    return *this;
}

string
IceSecurity::Ssl::ShutdownException::ice_name() const
{
    return "IceSecurity::Ssl::ShutdownException";
}

void
IceSecurity::Ssl::ShutdownException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity system shutdown exception";
}

Ice::Exception*
IceSecurity::Ssl::ShutdownException::ice_clone() const
{
    return new ShutdownException(*this);
}

void
IceSecurity::Ssl::ShutdownException::ice_throw() const
{
    throw *this;
}

/////////////////////////////////////
///////// ProtocolException /////////
/////////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::ProtocolException::ProtocolException(const char* errMessage, const char* file, int line) :
                                    ShutdownException(errMessage, file, line)
{
}

IceSecurity::Ssl::ProtocolException::ProtocolException(const ProtocolException& ex) :
                                    ShutdownException(ex)
{
}

IceSecurity::Ssl::ProtocolException&
IceSecurity::Ssl::ProtocolException::operator=(const ProtocolException& ex)
{
    ShutdownException::operator=(ex);
    return *this;
}

string
IceSecurity::Ssl::ProtocolException::ice_name() const
{
    return "IceSecurity::Ssl::ProtocolException";
}

void
IceSecurity::Ssl::ProtocolException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity system re-initialization exception";
}

Ice::Exception*
IceSecurity::Ssl::ProtocolException::ice_clone() const
{
    return new ProtocolException(*this);
}

void
IceSecurity::Ssl::ProtocolException::ice_throw() const
{
    throw *this;
}

///////////////////////////////////
////// CertificateException ///////
///////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::CertificateException::CertificateException(const char* errMessage, const char* file, int line) :
                                       ShutdownException(errMessage, file, line)
{
}

IceSecurity::Ssl::CertificateException::CertificateException(const CertificateException& ex) :
                                       ShutdownException(ex)
{
}

IceSecurity::Ssl::CertificateException&
IceSecurity::Ssl::CertificateException::operator=(const CertificateException& ex)
{
    ShutdownException::operator=(ex);
    return *this;
}

string
IceSecurity::Ssl::CertificateException::ice_name() const
{
    return "IceSecurity::CertificateException";
}

void
IceSecurity::Ssl::CertificateException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity system initialization exception";
}

Ice::Exception*
IceSecurity::Ssl::CertificateException::ice_clone() const
{
    return new CertificateException(*this);
}

void
IceSecurity::Ssl::CertificateException::ice_throw() const
{
    throw *this;
}

//////////////////////////////////////
////////// ContextException //////////
//////////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::OpenSSL::ContextException::ContextException(const char* errMessage, const char* file, int line) :
                                            SecurityException(errMessage, file, line)
{
}

IceSecurity::Ssl::OpenSSL::ContextException::ContextException(const ContextException& ex) :
                                            SecurityException(ex)
{
}

IceSecurity::Ssl::OpenSSL::ContextException&
IceSecurity::Ssl::OpenSSL::ContextException::operator=(const ContextException& ex)
{
    SecurityException::operator=(ex);
    return *this;
}

string
IceSecurity::Ssl::OpenSSL::ContextException::ice_name() const
{
    return "IceSecurity::Ssl::OpenSSL::ContextException";
}

void
IceSecurity::Ssl::OpenSSL::ContextException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nSSL System context exception";
}

Ice::Exception*
IceSecurity::Ssl::OpenSSL::ContextException::ice_clone() const
{
    return new ContextException(*this);
}

void
IceSecurity::Ssl::OpenSSL::ContextException::ice_throw() const
{
    throw *this;
}


