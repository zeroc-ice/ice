// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_PERMISSIONS_VERIFIER_PLUGIN
#define GLACIER2_PERMISSIONS_VERIFIER_PLUGIN

#include <Ice/Plugin.h>
#include <Glacier2/PermissionsVerifier.h>

namespace Glacier2
{

class GLACIER2_API CryptPermissionsVerifierPlugin :  public Ice::Plugin
{
public:
    
    virtual PermissionsVerifierPtr create(const std::string&) const = 0;
};
typedef IceUtil::Handle<CryptPermissionsVerifierPlugin> CryptPermissionsVerifierPluginPtr;

}

#endif
