// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/Config.h>

#ifdef ICE_OS_UWP

#include <IceSSL/SSLEngine.h>
#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <IceUtil/Shared.h>

#include <string>

IceUtil::Shared* IceSSL::upCast(IceSSL::UWPEngine* p) { return p; }

using namespace std;
using namespace IceSSL;

UWPEngine::UWPEngine(const Ice::CommunicatorPtr& communicator) : SSLEngine(communicator)
{
}

void
UWPEngine::initialize()
{
    lock_guard<mutex> lock(_mutex);
    if(_initialized)
    {
        return;
    }

    SSLEngine::initialize();

    const auto properties = communicator()->getProperties();

    //
    // Load CAs
    //
    //string ca = properties->getProperty("IceSSL.CAs");
    //if(!ca.empty())
    //{
    //  _ca = Certificate::load(ca);
    //}

    //
    // Load client certificate
    //
    const int passwordRetryMax = properties->getPropertyAsIntWithDefault("IceSSL.PasswordRetryMax", 3);
    setPassword(properties->getProperty("IceSSL.Password"));

    string certFile = properties->getProperty("IceSSL.CertFile");
    string findCert = properties->getProperty("IceSSL.FindCert");
    if(!certFile.empty())
    {
        _certificate = make_shared<IceSSL::Certificate>(importPersonalCertificate(
            certFile, 
            [this]()
            {
                return password(false);
            }, 
            getPasswordPrompt() != nullptr, 
            passwordRetryMax));
    }
    else if(!findCert.empty())
    {
        auto certs = findCertificates(properties->getPropertyWithDefault("IceSSL.CertStore", "My"), findCert);
        if(certs->Size == 0)
        {
            throw Ice::PluginInitializationException(__FILE__, __LINE__, "IceSSL: no certificates found");
        }
        _certificate = make_shared<IceSSL::Certificate>(certs->GetAt(0));
    }
    _initialized = true;
}

bool
UWPEngine::initialized() const
{
    return _initialized;
}

//shared_ptr<Certificate>
//UWPEngine::ca()
//{
//    return _ca;
//}

shared_ptr<Certificate>
UWPEngine::certificate()
{
    return _certificate;
}

void
UWPEngine::destroy()
{
}

#endif

