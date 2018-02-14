// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/NullPermissionsVerifier.h>
#include <Ice/Ice.h>

using namespace Ice;
using namespace std;

namespace
{

class NullPermissionsVerifier : public Glacier2::PermissionsVerifier
{
public:

    bool checkPermissions(const string&, const string&, string&, const Current&) const
    {
        return true;
    }
};

class NullSSLPermissionsVerifier : public Glacier2::SSLPermissionsVerifier
{
public:

    virtual bool
    authorize(const Glacier2::SSLInfo&, std::string&, const Ice::Current&) const
    {
        return true;
    }
};

class Init
{
public:
 
    Init(const CommunicatorPtr&, const string&, const vector<string>&);
     
private:
    
    string checkPermissionVerifier(const string&);
    void createObjects();

    const CommunicatorPtr _communicator;
    ObjectAdapterPtr _adapter;

    Identity _nullPVId;
    Identity _nullSSLPVId;
};

Init::Init(const CommunicatorPtr& communicator, const string& category, const vector<string>& props) :
    _communicator(communicator)
{
    _nullPVId.name = "NullPermissionsVerifier";
    _nullPVId.category = category;

    _nullSSLPVId.name = "NullSSLPermissionsVerifier";
    _nullSSLPVId.category = category;
    
    Ice::PropertiesPtr properties = _communicator->getProperties();
    for(vector<string>::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        string val = properties->getProperty(*p);
        if(!val.empty())
        {
            //
            // Check permission verifier proxy. It returns a non-empty
            // value with the new stringified proxy if the property
            // needs to be rewritten.
            //
            val = checkPermissionVerifier(val);
            if(!val.empty())
            {
                properties->setProperty(*p, val);
            }
        }
    }
}

string
Init::checkPermissionVerifier(const string& val)
{
    // Check if it's in proxy format
    try
    {
        ObjectPrx prx  = _communicator->stringToProxy(val);
        if(prx->ice_getIdentity() == _nullPVId || prx->ice_getIdentity() == _nullSSLPVId)
        {
            createObjects();
        }
    }
    catch(const ProxyParseException&)
    {
        // check if it's actually a stringified identity
        // (with typically missing " " because the category contains a space)
        
        if(val == _communicator->identityToString(_nullPVId))
        {
            createObjects();
            return _adapter->createProxy(_nullPVId)->ice_toString(); // Return valid proxy to rewrite the property
        }
        else if(val == _communicator->identityToString(_nullSSLPVId))
        {
            createObjects();
            return _adapter->createProxy(_nullSSLPVId)->ice_toString(); // Return valid proxy to rewrite the property
        }
        
        // Otherwise let the service report this incorrectly formatted proxy
    }
    return string();
}
   
void
Init::createObjects()
{
    if(!_adapter)
    {
        _adapter = _communicator->createObjectAdapter(""); // colloc-only adapter           
        _adapter->add(new NullPermissionsVerifier, _nullPVId);
        _adapter->add(new NullSSLPermissionsVerifier, _nullSSLPVId);
        _adapter->activate();
    }
}

}

namespace Glacier2Internal
{

void
setupNullPermissionsVerifier(const CommunicatorPtr& communicator, const string& category, const vector<string>& props)
{
    Init init(communicator, category, props);
}

}
