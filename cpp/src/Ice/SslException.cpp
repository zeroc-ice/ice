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

///////////////////////////////////
////////// InitException //////////
///////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::InitException::InitException(const char* errMessage, const char* file, int line) :
                                SecurityException(errMessage, file, line)
{
}

IceSecurity::Ssl::InitException::InitException(const InitException& ex) :
    SecurityException(ex)
{
}

IceSecurity::Ssl::InitException&
IceSecurity::Ssl::InitException::operator=(const InitException& ex)
{
    SecurityException::operator=(ex);
    return *this;
}

string
IceSecurity::Ssl::InitException::ice_name() const
{
    return "IceSecurity::InitException";
}

void
IceSecurity::Ssl::InitException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity system initialization exception";
}

Ice::Exception*
IceSecurity::Ssl::InitException::ice_clone() const
{
    return new InitException(*this);
}

void
IceSecurity::Ssl::InitException::ice_throw() const
{
    throw *this;
}

/////////////////////////////////////
////////// ReInitException //////////
/////////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::ReInitException::ReInitException(const char* errMessage, const char* file, int line) :
                                  SecurityException(errMessage, file, line)
{
}

IceSecurity::Ssl::ReInitException::ReInitException(const ReInitException& ex) :
                                  SecurityException(ex)
{
}

IceSecurity::Ssl::ReInitException&
IceSecurity::Ssl::ReInitException::operator=(const ReInitException& ex)
{
    SecurityException::operator=(ex);
    return *this;
}

string
IceSecurity::Ssl::ReInitException::ice_name() const
{
    return "IceSecurity::ReInitException";
}

void
IceSecurity::Ssl::ReInitException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity system re-initialization exception";
}

Ice::Exception*
IceSecurity::Ssl::ReInitException::ice_clone() const
{
    return new ReInitException(*this);
}

void
IceSecurity::Ssl::ReInitException::ice_throw() const
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
    return "IceSecurity::ShutdownException";
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

//////////////////////////////////////
////////// ContextException //////////
//////////////////////////////////////

//
// Public Methods
//

IceSecurity::Ssl::OpenSSL::ContextException::ContextException(const char* errMessage, const char* file, int line) :
                                            InitException(errMessage, file, line)
{
}

IceSecurity::Ssl::OpenSSL::ContextException::ContextException(const ContextException& ex) :
                                            InitException(ex)
{
}

IceSecurity::Ssl::OpenSSL::ContextException&
IceSecurity::Ssl::OpenSSL::ContextException::operator=(const ContextException& ex)
{
    InitException::operator=(ex);
    return *this;
}

string
IceSecurity::Ssl::OpenSSL::ContextException::ice_name() const
{
    return "IceSecurity::ContextException";
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


