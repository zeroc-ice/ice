// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/PluginI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/TransceiverI.h>

#include <Ice/LocalException.h>
#include <Ice/Object.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

//
// Plug-in factory function.
//
extern "C"
{

ICE_DECLSPEC_EXPORT Ice::Plugin*
createIceSSL(const CommunicatorPtr& communicator, const string& /*name*/, const StringSeq& /*args*/)
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
