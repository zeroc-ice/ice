//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/NullPermissionsVerifier.h>
#include <Ice/Ice.h>

using namespace Ice;
using namespace std;

namespace
{

class NullPermissionsVerifier final : public Glacier2::PermissionsVerifier
{
public:

    bool checkPermissions(string, string, string&, const Current&) const final
    {
        return true;
    }
};

class NullSSLPermissionsVerifier : public Glacier2::SSLPermissionsVerifier
{
public:

    bool authorize(Glacier2::SSLInfo, string&, const Ice::Current&) const final
    {
        return true;
    }
};

void setupObjectAdapter(
    const Ice::CommunicatorPtr& communicator,
    Ice::ObjectAdapterPtr& adapter)
{
    if (adapter == nullptr)
    {
        // Create collocated object adapter
        adapter = communicator->createObjectAdapter("");
        adapter->activate();
    }
}

}

namespace Glacier2Internal
{

void
setupNullPermissionsVerifier(
    const CommunicatorPtr& communicator,
    const string& category,
    const vector<string>& permissionsVerifierPropertyNames)
{
    const Ice::Identity nullPermissionsVerifierId { "NullPermissionsVerifier", category };
    const Ice::Identity nullSSLPermissionsVerifierId { "NullSSLPermissionsVerifier",  category };

    Ice::PropertiesPtr properties = communicator->getProperties();
    ObjectAdapterPtr adapter;

    shared_ptr<Glacier2::PermissionsVerifier> nullPermissionsVerifier;
    shared_ptr<Glacier2::SSLPermissionsVerifier> nullSSLPermissionsVerifier;

    for (const auto& propertyName : permissionsVerifierPropertyNames)
    {
        string val = properties->getProperty(propertyName);
        if (!val.empty())
        {
            try
            {
                ObjectPrx prx(communicator, val);
                if (prx->ice_getIdentity() == nullPermissionsVerifierId && nullPermissionsVerifier == nullptr)
                {
                    setupObjectAdapter(communicator, adapter);
                    nullPermissionsVerifier = make_shared<NullPermissionsVerifier>();
                    adapter->add(nullPermissionsVerifier, nullPermissionsVerifierId);
                }
                else if (prx->ice_getIdentity() == nullSSLPermissionsVerifierId && nullSSLPermissionsVerifier == nullptr)
                {
                    setupObjectAdapter(communicator, adapter);
                    nullSSLPermissionsVerifier = make_shared<NullSSLPermissionsVerifier>();
                    adapter->add(nullSSLPermissionsVerifier, nullSSLPermissionsVerifierId);
                }
            }
            catch(const ProxyParseException&)
            {
                // check if it's actually a stringified identity
                // (with typically missing " " because the category contains a space)

                if(val == communicator->identityToString(nullPermissionsVerifierId) && nullPermissionsVerifier == nullptr)
                {
                    setupObjectAdapter(communicator, adapter);
                    nullPermissionsVerifier = make_shared<NullPermissionsVerifier>();
                    ObjectPrx prx = adapter->add(nullPermissionsVerifier, nullPermissionsVerifierId);
                    properties->setProperty(propertyName, prx->ice_toString());
                }
                else if(val == communicator->identityToString(nullSSLPermissionsVerifierId) && nullSSLPermissionsVerifier == nullptr)
                {
                    setupObjectAdapter(communicator, adapter);
                    nullSSLPermissionsVerifier = make_shared<NullSSLPermissionsVerifier>();
                    ObjectPrx prx = adapter->add(nullSSLPermissionsVerifier, nullSSLPermissionsVerifierId);
                    properties->setProperty(propertyName, prx->ice_toString());
                }
                // Otherwise let the service report this incorrectly formatted proxy
            }
        }
    }
}

}
