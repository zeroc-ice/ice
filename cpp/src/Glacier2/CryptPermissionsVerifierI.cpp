// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/CryptPermissionsVerifierI.h>

#ifndef __APPLE__
#   include <openssl/des.h>
#endif

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::CryptPermissionsVerifierI::CryptPermissionsVerifierI(const map<string, string>& passwords) :
    _passwords(passwords)
{
}

bool
Glacier2::CryptPermissionsVerifierI::checkPermissions(
    const string& userId, const string& password, string&, const Current&) const
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, string>::const_iterator p = _passwords.find(userId);

    if(p == _passwords.end())
    {
        return false;
    }

    if(p->second.size() != 13) // Crypt passwords are 13 characters long.
    {
        return false;
    }

    char buff[14];
    string salt = p->second.substr(0, 2);
#if defined(__APPLE__)
    return p->second == crypt(password.c_str(), salt.c_str());
#else
    DES_fcrypt(password.c_str(), salt.c_str(), buff);
#endif
    return p->second == buff;
}
