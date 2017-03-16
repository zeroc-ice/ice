// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_UWP_ENGINE_H
#define ICESSL_UWP_ENGINE_H

#include <IceSSL/SSLEngine.h>
#include <IceSSL/UWP.h>

#include <mutex>

namespace IceSSL
{

namespace UWP
{

class SSLEngine : public IceSSL::SSLEngine
{
public:

    SSLEngine(const Ice::CommunicatorPtr&);

    virtual void initialize();
    virtual bool initialized() const;
    virtual void destroy();
    virtual IceInternal::TransceiverPtr
    createTransceiver(const InstancePtr&, const IceInternal::TransceiverPtr&, const std::string&, bool);
    virtual std::shared_ptr<IceSSL::UWP::Certificate> certificate();

private:

    std::shared_ptr<IceSSL::UWP::Certificate> _certificate;
};

} // UWP namespace end

} // IceSSL namespace end

#endif
