// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/CryptPermissionsVerifierI.h>
#include <openssl/opensslv.h>
#include <openssl/des.h>

// Ignore OS X OpenSSL deprecation warnings
#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
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
#if OPENSSL_VERSION_NUMBER >= 0x0090700fL
    DES_fcrypt(password.c_str(), salt.c_str(), buff);
#else
    des_fcrypt(password.c_str(), salt.c_str(), buff);
#endif
    return p->second == buff;
}
