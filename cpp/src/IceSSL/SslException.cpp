// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/LocalException.h>
#include <IceSSL/Exception.h>

using namespace std;
using namespace Ice;

void
IceSSL::SslException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    if(!_message.empty())
    {
        out << ":\n" << _message;
    }
}

void
IceSSL::ConfigurationLoadingException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::ConfigParseException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::ShutdownException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::ProtocolException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::CertificateVerificationException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::CertificateException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::CertificateSigningException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::CertificateSignatureException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::CertificateParseException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::PrivateKeyException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::PrivateKeyParseException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::CertificateVerifierTypeException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::OpenSSL::ContextException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::OpenSSL::ContextInitializationException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::OpenSSL::ContextNotConfiguredException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::OpenSSL::UnsupportedContextException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::OpenSSL::CertificateLoadException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::OpenSSL::PrivateKeyLoadException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::OpenSSL::CertificateKeyMatchException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}

void
IceSSL::OpenSSL::TrustedCertificateAddException::ice_print(ostream& out) const
{
    SslException::ice_print(out);
}
