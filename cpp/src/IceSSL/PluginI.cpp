// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/PluginI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/TransceiverI.h>

#include <Ice/LocalException.h>
#include <Ice/ConnectionI.h> // For implementation of getConnectionInfo.

using namespace std;
using namespace Ice;
using namespace IceSSL;

//
// Plug-in factory function.
//
extern "C"
{

ICE_DECLSPEC_EXPORT Ice::Plugin*
createIceSSL(const CommunicatorPtr& communicator, const string& name, const StringSeq& args)
{
    PluginI* plugin = new PluginI(communicator);
    return plugin;
}

}

//
// Plugin implementation.
//
IceSSL::PluginI::PluginI(const Ice::CommunicatorPtr& communicator)
{
    _instance = new Instance(communicator);
}

void
IceSSL::PluginI::initialize()
{
    _instance->initialize();
}

void
IceSSL::PluginI::destroy()
{
    _instance->destroy();
    _instance = 0;
}

void
IceSSL::PluginI::setContext(SSL_CTX* context)
{
    _instance->context(context);
}

SSL_CTX*
IceSSL::PluginI::getContext()
{
    return _instance->context();
}

void
IceSSL::PluginI::setCertificateVerifier(const CertificateVerifierPtr& verifier)
{
    _instance->setCertificateVerifier(verifier);
}

void
IceSSL::PluginI::setPasswordPrompt(const PasswordPromptPtr& prompt)
{
    _instance->setPasswordPrompt(prompt);
}

const char* IceSSL::ConnectionInvalidException::_name = "IceSSL::ConnectionInvalidException";

ConnectionInvalidException::ConnectionInvalidException(const char* file, int line, const string& r) :
    Exception(file, line),
    reason(r)
{
}

ConnectionInvalidException::~ConnectionInvalidException() throw()
{
}

string
ConnectionInvalidException::ice_name() const
{
    return _name;
}

Exception* 
ConnectionInvalidException::ice_clone() const
{
    return new ConnectionInvalidException(*this);
}

void
ConnectionInvalidException::ice_throw() const
{
    throw *this;
}

IceSSL::ConnectionInfo
IceSSL::getConnectionInfo(const ConnectionPtr& connection)
{
    Ice::ConnectionIPtr con = Ice::ConnectionIPtr::dynamicCast(connection);
    assert(con);

    //
    // Lock the connection directly. This is done because the only
    // thing that prevents the transceiver from being closed during
    // the duration of the invocation is the connection.
    //
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*con.get());
    IceInternal::TransceiverPtr transceiver = con->getTransceiver();
    if(!transceiver)
    {
        throw ConnectionInvalidException(__FILE__, __LINE__, "connection closed");
    }

    TransceiverIPtr ssltransceiver = TransceiverIPtr::dynamicCast(con->getTransceiver());
    if(!ssltransceiver)
    {
        throw ConnectionInvalidException(__FILE__, __LINE__, "not ssl connection");
    }

    try
    {
        return ssltransceiver->getConnectionInfo();
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << "couldn't get connection information:\n" << ex << endl;
        throw ConnectionInvalidException(__FILE__, __LINE__, os.str());
    }

    return ConnectionInfo(); // Required to prevent compiler warning on Solaris.
}
