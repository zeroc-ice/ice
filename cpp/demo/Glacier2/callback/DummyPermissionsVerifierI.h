// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DUMMY_PERMISSIONS_VERIFIER_I_H
#define DUMMY_PERMISSIONS_VERIFIER_I_H

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>

class DummyPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool checkPermissions(const std::string&, const std::string&, std::string&, const Ice::Current&) const
    {
	return true;
    }
};

#endif
