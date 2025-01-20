// Copyright (c) ZeroC, Inc.

#include "NullPermissionsVerifier.h"
#include "Glacier2/PermissionsVerifier.h"

using namespace Ice;
using namespace std;

namespace
{
    class NullPermissionsVerifier final : public Glacier2::PermissionsVerifier
    {
    public:
        bool checkPermissions(string, string, string&, const Current&) const final { return true; }
    };

    class NullSSLPermissionsVerifier : public Glacier2::SSLPermissionsVerifier
    {
    public:
        bool authorize(Glacier2::SSLInfo, string&, const Ice::Current&) const final { return true; }
    };
}

namespace Glacier2Internal
{
    void setupNullPermissionsVerifier(
        const CommunicatorPtr& communicator,
        const string& category,
        const vector<string>& permissionsVerifierPropertyNames)
    {
        const Ice::Identity nullPermissionsVerifierId{"NullPermissionsVerifier", category};
        const Ice::Identity nullSSLPermissionsVerifierId{"NullSSLPermissionsVerifier", category};

        const Ice::PropertiesPtr properties = communicator->getProperties();

        shared_ptr<Glacier2::PermissionsVerifier> nullPermissionsVerifier;
        shared_ptr<Glacier2::SSLPermissionsVerifier> nullSSLPermissionsVerifier;

        for (const auto& propertyName : permissionsVerifierPropertyNames)
        {
            string propertyValue = properties->getProperty(propertyName);
            if (!propertyValue.empty())
            {
                ObjectPrx prx(communicator, propertyValue);
                if (prx->ice_getIdentity() == nullPermissionsVerifierId && !nullPermissionsVerifier)
                {
                    nullPermissionsVerifier = make_shared<NullPermissionsVerifier>();
                }
                else if (prx->ice_getIdentity() == nullSSLPermissionsVerifierId && !nullSSLPermissionsVerifier)
                {
                    nullSSLPermissionsVerifier = make_shared<NullSSLPermissionsVerifier>();
                }
            }
        }

        if (nullPermissionsVerifier || nullSSLPermissionsVerifier)
        {
            // Create collocated object adapter for the null permissions verifier
            Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("");
            if (nullPermissionsVerifier)
            {
                adapter->add(std::move(nullPermissionsVerifier), nullPermissionsVerifierId);
            }

            if (nullSSLPermissionsVerifier)
            {
                adapter->add(std::move(nullSSLPermissionsVerifier), nullSSLPermissionsVerifierId);
            }
        }
    }
}
