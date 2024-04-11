//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IceSSL/SecureTransport.h"
#include "PluginI.h"
#include "SecureTransportEngine.h"

#include "Ice/Initialize.h"

using namespace Ice;
using namespace std;

IceSSL::CertificatePtr
IceSSL::Certificate::load(const std::string& file)
{
    return IceSSL::SecureTransport::Certificate::load(file);
}

IceSSL::CertificatePtr
IceSSL::Certificate::decode(const std::string& encoding)
{
    return IceSSL::SecureTransport::Certificate::decode(encoding);
}
