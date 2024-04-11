//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "PluginI.h"
#include "SChannelEngine.h"

#include "Ice/Initialize.h"

using namespace std;

IceSSL::CertificatePtr
IceSSL::Certificate::load(const std::string& file)
{
    return IceSSL::SChannel::Certificate::load(file);
}

IceSSL::CertificatePtr
IceSSL::Certificate::decode(const std::string& encoding)
{
    return IceSSL::SChannel::Certificate::decode(encoding);
}
