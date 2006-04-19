// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Instance.h>
#include <EndpointI.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Logger.h>
#include <Ice/Properties.h>
#include <Ice/ProtocolPluginFacade.h>

#include <openssl/err.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

void IceInternal::incRef(IceSSL::Instance* p) { p->__incRef(); }
void IceInternal::decRef(IceSSL::Instance* p) { p->__decRef(); }

IceSSL::Instance::Instance(const CommunicatorPtr& communicator)
{
    __setNoDelete(true);

    PropertiesPtr properties = communicator->getProperties();

    _facade = IceInternal::getProtocolPluginFacade(communicator);
    _securityTraceLevel = properties->getPropertyAsInt("IceSSL.Trace.Security");
    _securityTraceCategory = "Security";

    //
    // Create the context.
    //
    // If IceSSL.DelayInit=1, postpone the creation of the context until
    // the application manually initializes the plugin.
    //
    if(properties->getPropertyAsInt("IceSSL.DelayInit") == 0)
    {
	_context = new Context(this, 0);
    }

    //
    // Register the endpoint factory.
    //
    _facade->addEndpointFactory(new EndpointFactoryI(this));

    __setNoDelete(false);
}

void
IceSSL::Instance::initialize(SSL_CTX* context)
{
    if(_context)
    {
	SecurityException ex(__FILE__, __LINE__);
	ex.reason = "plugin is already initialized";
	throw ex;
    }
    else
    {
	_context = new Context(this, context);
    }
}

void
IceSSL::Instance::setCertificateVerifier(const CertificateVerifierPtr& verifier)
{
    _verifier = verifier;
}

void
IceSSL::Instance::setPasswordPrompt(const PasswordPromptPtr& prompt)
{
    _prompt = prompt;
}

CommunicatorPtr
IceSSL::Instance::communicator() const
{
    return _facade->getCommunicator();
}

string
IceSSL::Instance::defaultHost() const
{
    return _facade->getDefaultHost();
}

int
IceSSL::Instance::networkTraceLevel() const
{
    return _facade->getNetworkTraceLevel();
}

string
IceSSL::Instance::networkTraceCategory() const
{
    return _facade->getNetworkTraceCategory();
}

int
IceSSL::Instance::securityTraceLevel() const
{
    return _securityTraceLevel;
}

string
IceSSL::Instance::securityTraceCategory() const
{
    return _securityTraceCategory;
}

ContextPtr
IceSSL::Instance::context() const
{
    if(!_context)
    {
	PluginInitializationException ex(__FILE__, __LINE__);
	ex.reason = "IceSSL: plugin is not fully initialized";
	throw ex;
    }
    return _context;
}

CertificateVerifierPtr
IceSSL::Instance::certificateVerifier() const
{
    return _verifier;
}

PasswordPromptPtr
IceSSL::Instance::passwordPrompt() const
{
    return _prompt;
}

string
IceSSL::Instance::sslErrors() const
{
    ostringstream ostr;

    const unsigned long threadId = CRYPTO_thread_id();

    const char* file;
    const char* data;
    int line;
    int flags;
    unsigned long err;
    int count = 0;
    while((err = ERR_get_error_line_data(&file, &line, &data, &flags)) != 0)
    {
	if(count > 0)
	{
	    ostr << endl;
	}

	if(_securityTraceLevel > 0)
	{
	    if(count > 0)
	    {
		ostr << endl;
	    }

	    char buf[200];
	    ERR_error_string_n(err, buf, sizeof(buf));

	    ostr << "Thread ID: " << threadId << endl;
	    ostr << "Error #:   " << err << endl;
	    ostr << "Message:   " << buf << endl;
	    ostr << "Location:  " << file << ", " << line;
	    if(flags & ERR_TXT_STRING)
	    {
		ostr << endl;
		ostr << "Data:      " << data;
	    }
	}
	else
	{
	    const char* reason = ERR_reason_error_string(err);
	    ostr << (reason == NULL ? "unknown reason" : reason);
	    if(flags & ERR_TXT_STRING)
	    {
		ostr << ": " << data;
	    }
	}

	++count;
    }

    ERR_clear_error();

    return ostr.str();
}

void
IceSSL::Instance::destroy()
{
    _facade = 0;
    _context = 0;
}
