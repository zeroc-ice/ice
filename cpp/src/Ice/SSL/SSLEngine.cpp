//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SSLEngine.h"
#include "../Instance.h"
#include "Ice/Communicator.h"
#include "Ice/LocalException.h"
#include "Ice/Logger.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Ice/SSL/ConnectionInfo.h"
#include "IceUtil/StringUtil.h"
#include "TrustManager.h"

#include <string>

using namespace std;
using namespace Ice;
using namespace IceUtil;
using namespace Ice::SSL;

Ice::SSL::SSLEngine::SSLEngine(const IceInternal::InstancePtr& instance)
    : _instance(instance),
      _trustManager(make_shared<TrustManager>(instance)),
      _revocationCheckCacheOnly(false),
      _revocationCheck(0)
{
}

Ice::SSL::SSLEngine::~SSLEngine() {}

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
    const string propPrefix = "IceSSL.";
    const PropertiesPtr properties = getProperties();

    // CheckCertName determines whether we compare the name in a peer's certificate against its hostname.
    _checkCertName = properties->getPropertyAsIntWithDefault(propPrefix + "CheckCertName", 0) > 0;

    // CheckCertName > 1 enables SNI, the SNI extension applies to client connections, indicating the hostname to the
    // server (must be DNS hostname, not an IP address).
    _serverNameIndication = properties->getPropertyAsIntWithDefault(propPrefix + "CheckCertName", 0) > 1;

    // VerifyPeer determines whether certificate validation failures abort a connection.
    _verifyPeer = properties->getPropertyAsIntWithDefault(propPrefix + "VerifyPeer", 2);

    if (_verifyPeer < 0 || _verifyPeer > 2)
    {
        throw InitializationException(
            __FILE__,
            __LINE__,
            "SSL transport: invalid value for " + propPrefix + "VerifyPeer");
    }

    _securityTraceLevel = properties->getPropertyAsInt("IceSSL.Trace.Security");
    _securityTraceCategory = "Security";

    const_cast<bool&>(_revocationCheckCacheOnly) =
        properties->getPropertyAsIntWithDefault("IceSSL.RevocationCheckCacheOnly", 1) > 0;
    const_cast<int&>(_revocationCheck) = properties->getPropertyAsIntWithDefault("IceSSL.RevocationCheck", 0);
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

bool
Ice::SSL::SSLEngine::getServerNameIndication() const
{
    return _serverNameIndication;
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
