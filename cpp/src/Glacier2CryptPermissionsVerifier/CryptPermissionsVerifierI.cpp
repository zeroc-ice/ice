// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/PermissionsVerifier.h>
#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>

#include <IceUtil/FileUtil.h>
#include <IceUtil/StringUtil.h>

#ifndef __APPLE__
#   include <openssl/des.h>
#endif

using namespace std;
using namespace Ice;
using namespace Glacier2;

namespace
{

class CryptPermissionsVerifierI : public PermissionsVerifier, public IceUtil::Mutex
{
public:

    CryptPermissionsVerifierI(const map<string, string>&);

    virtual bool checkPermissions(const string&, const string&, string&, const Ice::Current&) const;

private:

    const map<string, string> _passwords;
};

class CryptPermissionsVerifierPlugin : public Ice::Plugin
{
public:

    CryptPermissionsVerifierPlugin(const CommunicatorPtr&);
    
    virtual void initialize();
    virtual void destroy();

private:
    
    CommunicatorPtr _communicator;
};


map<string, string>
retrievePasswordMap(const string& file)
{
    IceUtilInternal::ifstream passwordFile(file);
    if(!passwordFile)
    {
        string err = IceUtilInternal::lastErrorToString();
        throw Ice::InitializationException(__FILE__, __LINE__, "cannot open `" + file + "' for reading: " + err);
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
    return passwords;
}

CryptPermissionsVerifierI::CryptPermissionsVerifierI(const map<string, string>& passwords) :
    _passwords(passwords)
{
}

bool
CryptPermissionsVerifierI::checkPermissions(const string& userId, const string& password, string&, const Current&) const
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


CryptPermissionsVerifierPlugin::CryptPermissionsVerifierPlugin(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
CryptPermissionsVerifierPlugin::initialize()
{
    const string prefix = "Glacier2CryptPermissionsVerifier.";
    const PropertyDict props = _communicator->getProperties()->getPropertiesForPrefix(prefix);
  
    if(!props.empty())
    {
        ObjectAdapterPtr adapter = _communicator->createObjectAdapter(""); // colloc-only adapter
        
        // Each prop represents a property to set + the associated password file
        
        for(PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
        {
            string name = p->first.substr(prefix.size());
            Identity id;
            id.name = IceUtil::generateUUID();
            id.category = "Glacier2CryptPermissionsVerifier";
            ObjectPrx prx = adapter->add(new CryptPermissionsVerifierI(retrievePasswordMap(p->second)), id);
            _communicator->getProperties()->setProperty(name, _communicator->proxyToString(prx));  
        }
        
        adapter->activate();
    }
}

void
CryptPermissionsVerifierPlugin::destroy()
{
}

}

//
// Plug-in factory function.
//
extern "C"
{

ICE_DECLSPEC_EXPORT Ice::Plugin*
createCryptPermissionsVerifier(const CommunicatorPtr& communicator, const string& name, const StringSeq& args)
{
    if(args.size() > 0)
    {
        Error out(communicator->getLogger());
        out << "Plugin " << name << ": too many arguments";
        return 0;
    }
    
    return new CryptPermissionsVerifierPlugin(communicator);
}

}
