// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <SessionI.h>

using namespace std;
using namespace Ice;

bool
DummyPermissionsVerifierI::checkPermissions(const string& userId, const string& password, string&,
					    const Current&) const
{
    cout << "verified user-id `" << userId << "' with password `" << password << "'" << endl;
    return true;
}
