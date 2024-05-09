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
    const PropertiesPtr properties = getProperties();

    // CheckCertName determines whether we compare the name in a peer's certificate against its hostname.
    _checkCertName = properties->getIcePropertyAsInt("IceSSL.CheckCertName") > 0;

    // CheckCertName > 1 enables SNI, the SNI extension applies to client connections, indicating the hostname to the
    // server (must be DNS hostname, not an IP address).
    _serverNameIndication = properties->getIcePropertyAsInt("IceSSL.CheckCertName") > 1;

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
Ice::SSL::SSLEngine::verifyPeerCertName(const ConnectionInfoPtr& info, const string& address) const
{
    // For an outgoing connection, we compare the proxy address (if any) against fields in the server's certificate
    // (if any).
    if (_checkCertName && !info->certs.empty() && !address.empty())
    {
        const CertificatePtr cert = info->certs[0];

        // Extract the IP addresses and the DNS names from the subject alternative names.
        vector<pair<int, string>> subjectAltNames = cert->getSubjectAlternativeNames();
        vector<string> ipAddresses;
        vector<string> dnsNames;
        for (vector<pair<int, string>>::const_iterator p = subjectAltNames.begin(); p != subjectAltNames.end(); ++p)
        {
            if (p->first == AltNAmeIP)
            {
                ipAddresses.push_back(IceUtilInternal::toLower(p->second));
            }
            else if (p->first == AltNameDNS)
            {
                dnsNames.push_back(IceUtilInternal::toLower(p->second));
            }
        }

        bool certNameOK = false;
        string addrLower = IceUtilInternal::toLower(address);
        bool isIpAddress = IceInternal::isIpAddress(address);

        // If address is an IP address, compare it to the subject alternative names IP address
        if (isIpAddress)
        {
            certNameOK = find(ipAddresses.begin(), ipAddresses.end(), addrLower) != ipAddresses.end();
        }
        else
        {
            // If subjectAlt is empty compare it to the subject CN, otherwise compare it to the to the subject alt
            // name dnsNames.
            if (dnsNames.empty())
            {
                DistinguishedName d = cert->getSubjectDN();
                string dn = IceUtilInternal::toLower(string(d));
                string cn = "cn=" + addrLower;
                string::size_type pos = dn.find(cn);
                if (pos != string::npos)
                {
                    // Ensure we match the entire common name.
                    certNameOK = (pos + cn.size() == dn.size()) || (dn[pos + cn.size()] == ',');
                }
            }
            else
            {
                certNameOK = find(dnsNames.begin(), dnsNames.end(), addrLower) != dnsNames.end();
            }
        }

        if (!certNameOK)
        {
            ostringstream ostr;
            ostr << "SSL transport: ";
            if (_verifyPeer > 0)
            {
                ostr << "ignoring ";
            }
            ostr << "certificate verification failure " << (isIpAddress ? "IP address mismatch" : "Hostname mismatch");
            string msg = ostr.str();
            if (_securityTraceLevel >= 1)
            {
                Trace out(getLogger(), _securityTraceCategory);
                out << msg;
            }

            if (_verifyPeer > 0)
            {
                throw SecurityException(__FILE__, __LINE__, msg);
            }
        }
    }
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
