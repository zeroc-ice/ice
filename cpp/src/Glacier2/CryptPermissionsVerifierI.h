// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>

namespace Glacier2
{

class CryptPermissionsVerifierI : public PermissionsVerifier, public IceUtil::Mutex
{
public:

    CryptPermissionsVerifierI(const std::map<std::string, std::string>&);

    virtual bool checkPermissions(const std::string&, const std::string&, std::string&, const Ice::Current&) const;

private:

    const std::map<std::string, std::string> _passwords;
};

}
