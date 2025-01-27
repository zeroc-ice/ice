// Copyright (c) ZeroC, Inc.

#include "SSLEngine.h"
#include "../Instance.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Ice/Properties.h"
#include "Ice/SSL/ConnectionInfo.h"
#include "TrustManager.h"

#include <string>

using namespace std;
using namespace Ice;
using namespace Ice::SSL;

Ice::SSL::SSLEngine::SSLEngine(const IceInternal::InstancePtr& instance)
    : _instance(instance),
      _trustManager(make_shared<TrustManager>(instance))
{
}

Ice::SSL::SSLEngine::~SSLEngine() = default;

Ice::LoggerPtr
Ice::SSL::SSLEngine::getLogger() const
{
    return _instance->initializationData().logger;
}

Ice::PropertiesPtr
Ice::SSL::SSLEngine::getProperties() const
{
    return _instance->initializationData().properties;
}

Ice::InitializationData
Ice::SSL::SSLEngine::getInitializationData() const
{
    return _instance->initializationData();
}

void
Ice::SSL::SSLEngine::initialize()
{
    const PropertiesPtr properties = getProperties();

    // CheckCertName determines whether we compare the name in a peer's certificate against its hostname.
    _checkCertName = properties->getIcePropertyAsInt("IceSSL.CheckCertName") > 0;

    // VerifyPeer determines whether certificate validation failures abort a connection.
    _verifyPeer = properties->getIcePropertyAsInt("IceSSL.VerifyPeer");

    if (_verifyPeer < 0 || _verifyPeer > 2)
    {
        throw InitializationException(__FILE__, __LINE__, "SSL transport: invalid value for IceSSL.VerifyPeer");
    }

    _securityTraceLevel = properties->getIcePropertyAsInt("IceSSL.Trace.Security");
    _securityTraceCategory = "Security";

    const_cast<bool&>(_revocationCheckCacheOnly) =
        properties->getIcePropertyAsInt("IceSSL.RevocationCheckCacheOnly") > 0;
    const_cast<int&>(_revocationCheck) = properties->getIcePropertyAsInt("IceSSL.RevocationCheck");
}

void
Ice::SSL::SSLEngine::verifyPeer(const ConnectionInfoPtr& info) const
{
    if (!_trustManager->verify(info))
    {
        string msg = string(info->incoming ? "incoming" : "outgoing") + " connection rejected by trust manager";
        if (_securityTraceLevel >= 1)
        {
            getLogger()->trace(_securityTraceCategory, msg);
        }
        throw SecurityException(__FILE__, __LINE__, msg);
    }
}

bool
Ice::SSL::SSLEngine::getCheckCertName() const
{
    return _checkCertName;
}

int
Ice::SSL::SSLEngine::getVerifyPeer() const
{
    return _verifyPeer;
}

int
Ice::SSL::SSLEngine::securityTraceLevel() const
{
    return _securityTraceLevel;
}

std::string
Ice::SSL::SSLEngine::securityTraceCategory() const
{
    return _securityTraceCategory;
}

bool
Ice::SSL::SSLEngine::getRevocationCheckCacheOnly() const
{
    return _revocationCheckCacheOnly;
}

int
Ice::SSL::SSLEngine::getRevocationCheck() const
{
    return _revocationCheck;
}
