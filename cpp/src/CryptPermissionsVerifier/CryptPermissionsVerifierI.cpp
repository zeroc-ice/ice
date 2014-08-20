// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/FileUtil.h>

#include <Ice/LocalException.h>

#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/CryptPermissionsVerifierPlugin.h>

#ifndef __APPLE__
#   include <openssl/des.h>
#endif

using namespace std;
using namespace Ice;
using namespace Glacier2;

namespace
{

class PluginI : public CryptPermissionsVerifierPlugin
{
public:
    
    virtual void initialize();
    virtual void destroy();
    
    virtual PermissionsVerifierPtr create(const string&) const;
};

class CryptPermissionsVerifierI : public PermissionsVerifier, public IceUtil::Mutex
{
public:

    CryptPermissionsVerifierI(const map<string, string>&);

    virtual bool checkPermissions(const string&, const string&, string&, const Ice::Current&) const;

private:

    const map<string, string> _passwords;
};

void PluginI::initialize()
{
}

void PluginI::destroy()
{
}

PermissionsVerifierPtr
PluginI::create(const string& passwordsProperty) const
{
    IceUtilInternal::ifstream passwordFile(passwordsProperty);
    if(!passwordFile)
    {
        string err = strerror(errno);
        throw Ice::InitializationException(__FILE__, __LINE__, 
                                           "cannot open `" + passwordsProperty + "' for reading: " + err);
    }
    map<string, string> passwords;

    while(true)
    {
        string userId;
        passwordFile >> userId;
        if(!passwordFile)
        {
            break;
        }

        string password;
        passwordFile >> password;
        if(!passwordFile)
        {
            break;
        }

        assert(!userId.empty());
        assert(!password.empty());
        passwords.insert(make_pair(userId, password));
    }
    return new CryptPermissionsVerifierI(passwords);
}

CryptPermissionsVerifierI::CryptPermissionsVerifierI(const map<string, string>& passwords) :
    _passwords(passwords)
{
}

bool
CryptPermissionsVerifierI::checkPermissions(const string& userId, const string& password, string&, 
                                            const Current&) const
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
#   if OPENSSL_VERSION_NUMBER >= 0x0090700fL
    DES_fcrypt(password.c_str(), salt.c_str(), buff);
#   else
    des_fcrypt(password.c_str(), salt.c_str(), buff);
#   endif
#endif
    return p->second == buff;
}

}

//
// Plug-in factory function.
//
extern "C"
{

ICE_DECLSPEC_EXPORT Ice::Plugin*
createCryptPermissionsVerifier(const CommunicatorPtr&, const string&, const StringSeq&)
{
    return new PluginI();
}

}
