//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Initialize.h"
#include "IceSSL/OpenSSL.h"
#include "OpenSSLEngine.h"
#include "PluginI.h"

using namespace std;

//
// The following functions are defined only when OpenSSL is the default
// implementation. In Windows the default implementation is always
// SChannel.
//
IceSSL::CertificatePtr
IceSSL::Certificate::load(const std::string& file)
{
    return IceSSL::OpenSSL::Certificate::load(file);
}

IceSSL::CertificatePtr
IceSSL::Certificate::decode(const std::string& encoding)
{
    return IceSSL::OpenSSL::Certificate::decode(encoding);
}
